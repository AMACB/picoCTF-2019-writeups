# Time's Up

> Time waits for no one. Can you solve this before time runs out? [times-up](times-up), located in the directory at ``/problems/time-s-up_1_7d4f79c3df3e1b044801573eea5722be``.

> Hints: Can you interact with the program using a script?

Upon running the program, we are treated with the very fast run of text.
```
Challenge: (((((-2126372103) - (225877768)) + ((-1551135096) + (1471356896))) + (((686165910) + (-1245290406)) + ((-995861592) + (1815320614)))) - ((((-940484744) + (-1249322894)) + ((284597278) + (1704808354))) - (((462451468) + (833169926)) + ((-1467212263) + (-2138574505)))))
Setting alarm...
Solution? Alarm clock
```
Along with the challenge name, description, and key phrase ``Alarm clock``, it looks like the goal of this question is to solve the challenge before some alarm finishes.

And as the hint suggests, this can be easily accomplished using a script.
```python3
from pwn import *
from time import sleep

# Run in /problems/time-s-up_1_7d4f79c3df3e1b044801573eea5722be
p = process('./times-up')

# Receive the challenge
chall = p.recv(4096)
# Chop off the unnecessary text
chall = chall[len('Challenge: '):chall.index('\n')]
# eval and send it through
p.sendline(str(eval(chall)))

print p.recv(4096) # Here is the flag:
print p.recv(4096) # Actual flag
```
Running the above gives us our flag.
> ``picoCTF{Gotta go fast. Gotta go FAST. #3daa579a}``
