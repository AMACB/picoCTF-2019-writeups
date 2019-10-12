// This is the pseudocode of the binary
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
	/* No buffering */
	setvbuf(stdin,NULL,_IONBF,0);
	setvbuf(stdout,NULL,_IONBF,0);
	setvbuf(stderr,NULL,_IONBF,0);

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
