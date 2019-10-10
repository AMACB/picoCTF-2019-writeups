# Ghost_Diary

> Try writing in this [ghost diary](ghostdiary). It's also found in `/problems/ghost-diary_4_e628b10cf58ea41692460c7ea1e05578` on the shell server.

We are given a stripped 64-bit binary called `ghostdiary` as well as a directory on the shell server. Checking the libc version on the server gives us version `2.27` (which is the standard version for Ubuntu systems as of writing). Let's also do a `checksec`:
```
[*] '/.../ghostdiary'
    Arch:     amd64-64-little
    RELRO:    Full RELRO
    Stack:    Canary found
    NX:       NX enabled
    PIE:      PIE enabled

```
It looks like everything is enabled, so we have quite the task ahead of us. After running, we are given a menu with the following options: `New page`, `Talk with ghost`, `Listen to ghost`, `Burn the page`, and `Go to sleep`. We suspect that these operations correspond to a fairly standard heap problem with `create`, `edit`, `read`, `delete`, and `exit`. Let's now reverse the binary with Ghidra, `objdump`, and `gdb`:
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

struct page {
	char* content;
	unsigned int size;
};

struct page diary[20];

void edit_page_input(struct page p) {
	unsigned int i = 0;
	if (p.size != 0) {
		for (; i != p.size; i++) {
			char c;
			if (read(0,&c,1) != 1) {
				puts("read error");
				exit(-1);
			}
			if (c == '\n') break;
			p.content[i] = c;
		}
		p.content[i] = '\x00'; // OFF BY ONE BUG
	}
}
void create_page() {
	char* new_page;
	unsigned int size;
	int choice;
	unsigned int i = 0;
	for (; i < 20 && (diary[i].content != NULL); i++);
	if (i == 20) {
		puts("Buy new book");
	} else {
		puts("1. Write on one side?");
		puts("2. Write on both sides?");
        while (1) {
            printf("> ");
            
            scanf("%d",&choice);
            if (choice == 1) {
                printf("size: ");
                scanf("%d",&size);
                if (size > 240) {
                    puts("too big to fit in a page");
                    continue;
                } else {
                    break;
                }
            } else if (choice == 2) {
                printf("size: ");
                scanf("%d",&size);
                if (size < 272) {
                    puts("don\'t waste pages -_-");
                } else if (size > 480) {
                    puts("can you not write that much?");
                } else {
                    break;
                }
            } else {
                return;
            }
        }
		new_page = malloc(size);
		diary[i].content = new_page;
		if (diary[i].content == NULL) {
			puts("oh noooooooo!! :(");
		} else {
			diary[i].size = size;
			printf("page #%d\n",i);
		}
	}
}
void edit_page() {
	unsigned int num;
	printf("Page: ");
	scanf("%d",&num);
	printf("Content: ");
	if ((num < 20) && (diary[num].content != NULL)) {
		edit_page_input(diary[num]);
	}
}
void print_page() {
	unsigned int num;
	printf("Page: ");
	scanf("%d",&num);
	printf("Content: ");
	if ((num < 20) && (diary[num].content != NULL)) {
		puts(diary[num].content);
	}
}
void delete_page() {
	unsigned int num;
	printf("Page: ");
	scanf("%d",&num);
	if ((num < 20) && (diary[num].content != NULL)) {
		free(diary[num].content);
		diary[num].content = NULL;
	}
}
void menu() {
	puts("1. New page in diary");
	puts("2. Talk with ghost");
	puts("3. Listen to ghost");
	puts("4. Burn the page");
	puts("5. Go to sleep");
	printf("> ");
}

void exitnow() {exit(-1);}
int main() {
	char tmp;
	int choice;
	setvbuf(stdin,NULL,_IONBF,0);
	setvbuf(stdout,NULL,_IONBF,0);
	setvbuf(stderr,NULL,_IONBF,0);
	alarm(0x3c);
	signal(SIGALRM,exitnow);
	puts("-=-=-=[[Ghost Diary]]=-=-=-");
	while (1) {
		menu();
		scanf("%d",&choice);
		do { tmp = getchar(); } while (tmp != '\n');
		switch(choice) {
		case 1:
			create_page();
			break;
		case 2:
			edit_page();
			break;
		case 3:
			print_page();
			break;
		case 4:
			delete_page();
			break;
		case 5:
			puts("bye human!");
			return 0;
		default:
			puts("Invalid choice");
		}
	}
}
```
In `create_page`, we can allocate only chunks of size `n` where `0 <= n <= 240` or `272 <= n <= 480`. In `delete_page`, we notice that the corresponding pointer is `NULL`ed out, so it is secure from a standard double free and use-after-free. However, the `edit_page` has an off-by-one `NULL` byte vulnerability. If we `edit_page` and fill the input, we can get the `NULL` byte to overflow into an adjacent chunk, allowing us to overwrite part of the size and the `PREV_INUSE` bit of that chunk.

Now that we know the basis of our exploit, let's get into the details. We remember that libc 2.27 has `tcache` enabled with minimal security checks, meaning we can easily [convert a UAF into an arbitrary write](https://github.com/shellphish/how2heap/blob/master/glibc_2.26/tcache_poisoning.c). Of course, the only bug we have is the `NULL` overflow, but we can use this to overwrite a `PREV_INUSE` bit to force a consolidation