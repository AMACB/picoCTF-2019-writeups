# Time's Up, for the Last Time!

> You've solved things fast. You've solved things faster! Now do the impossible. [times-up-one-last-time](times-up-one-last-time), located in the directory at /problems/time-s-up--for-the-last-time-_1_a7830af9d51a361ee5d3b9eece69c22f.

> Hints: Some times, if some approach seems impossible, it means a different perspective might be needed. Is there anything interesting about how the program behaves?

Upon opening the program, we are greeted with the following by-now-familiar set of text.
```
Challenge: (((((931684457) x (-141199422)) | ((-2112115878) - (150425815))) - ((((554752943) | (409636616)) * ((-551976236) o (446132558))) * ((-216677243) x (462203675)))) - ((((-1802665975) r (-2089095274)) + ((-1538299143) o (1410099799))) + (((767697014) | (75366479)) | ((656800643) | (-1002949848)))))
Setting alarm...
Solution? Alarm clock
```
This time around there appear to be a bunch of random characters to evaluate through. To figure these out, we will actually have to dig  into the program this time.

Decompiling the program (say, with ghidra) and poking around for a bit reveals the main function as follows.
```c
int main() {
	init_randomness();
	printf("Challenge: ");
	generate_challenge();
	putchar('\n');
	fflush(stdout);
	puts("Setting alarm...");
	fflush(stdout);
	ualarm(10,0);
	printf("Solution? ");
	scanf("%lld", &guess);
	if (guess == result) {
		puts("Congrats! Here is the flag!");
		system("/bin/cat flag.txt");
	}
	else {
		puts("Nope!");
	}
	return 0;
}
```
The key line for the alarm, of course, is ``ualarm(10,0)``, which sets the alarm to go off in no less than *10 microseconds*. The python script for the previous challenge was a bit iffy on its timing performance, and we can go back to check that the alarm was set for 200 microseconds, so it appears pretty impossible for any script to solve this one.

We could sift through the challenge generation, but it is largely unimportant; the real key to solving this one is finding away to deal with the time constraint. To show how extreme 10 microseconds is, we can try to manually insert fixed input into the program on the shell, and we still never get past the alarm.
```bash
user@pico-2019-shell1:~$ /problems/time-s-up--for-the-last-time-[...]/times-up-again <<< '\n'
Challenge: ...
Setting alarm...
Alarm clock
```
As a sidenote, while the above never succeeds on the server, it does occasionally when run locally, and in fact, it sometimes is even correct, a shocking proportion of the time (say, more than 1 in 20).
```bash
$ ./times-up-one-last-time <<< '\n'
Challenge: ...
Setting alarm...
Solution? Congrats! Here is the flag!
Alarm clock
```
So if it is impossible to even get input into the program in 10 microseconds, then the only way to solve this challenge will have to bypass the alarm, which signals SIGALRM. Upon hint from an admin, we realized that (roughly) the same techniques that ignore the alarms in gdb can also ignore the time alarm when communicating with the program normally.

The following C program opens a session with the file in which the alarm is ignored.
```c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int main() {
	signal(SIGALRM, SIG_IGN);
	system("./times-up-one-last-time");
}
```
Now that we're able to get input into the program, we can return to the idea of literally only inserting a new line and sometimes retrieving the flag. (There are interesting reasons this occurs, but they largely don't matter. In short, the answer is frequently 0 depending on the structure of the challenge, and no input as in ``\n`` is read in as 0.) Running the above C program enough times with just spamming an input of ``0`` will eventually retrieve the flag. Here's a bash for loop that will do the job.
```bash
/problems/time-s-up--for-the-last-time-[...]$ for i in {0..1000..1}; do (exploit <<< '\n') | grep "picoCTF"; done
```
And so we have our flag.
> ``picoCTF{And now you can hack time! #2e0a37d1}``
