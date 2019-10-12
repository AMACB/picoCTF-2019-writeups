// This file contains our pseudocode for the program.
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
