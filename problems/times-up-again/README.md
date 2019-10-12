# Time's Up Again

> Previously you solved things fast. Now you've got to go faster. Much faster. Can you solve *this one* before time runs out? [times-up-again](times-up-again), located in the directory at ``/problems/time-s-up--again-_0_ba1fe87bd4905d3f34eb83d66f907fe0``.

> Hints: Sometimes, scripts are just too slow. You've got to have much more control.

In spite of the hint, it turns out scripts are fast enough, with some optimizations. Upon opening the challenge, we are greeted with a very similar run of text.
```
Challenge: (((((2028325709) + (674163678)) * ((1611206565) * (215980341))) * (((824932500) * (-742149897)) + ((954247263) + (-2098073143)))) - ((((-639264993) + (-1377601293)) + ((-1704616670) - ((-473516344) + (893927024)))) * (((-1236496098) * (1228643134)) * ((652120780) - (1679126213)))))
Setting alarm...
Solution? Alarm clock
```
In an attempt to not actually solve the problem, we instead first attempt to just optimize the script. First, we use ``subprocess`` instead of ``pwntools`` if for no other reason than it allows us to call ``p.stdout.readline()`` for the first line and then immediately send back the response. Second, we turn on our code golf and magic numbers and take in the input to the challenge and output it back in the same line in order to avoid setting variables. Some of this might have been optimized away by the compiler, but ``python3`` generally does not try too hard to speed up programs during interpretation.

But finally, we have to apply ``% (1 << 64)`` to the answer. The reason for this is a bit subtle, but the original program was likely written in a language that does not implicitly support arbitrary-percision integers, such as the ``python3`` we are scripting in. And checking the binary, it is only 64-bit:
<pre>
times-up-again: setgid ELF <b>64-bit</b> LSB shared object ...
</pre>
Thus, any integer we want to pass into the program we should turn back into 64-bit, and the easiest way to do this is by manually modding out the more significant bits. Even though this might give us a positive answer when the program thinks it should be negative, or give us a negative answer when the program wants it positive, we'll get it right about half the time.

With all of that said, here is the script. As promised, it uses ``subprocess`` and is not at all written for readability. Comments were added after.
```python3
import subprocess

# Run in /problems/time-s-up--again-_0_ba1fe87bd4905d3f34eb83d66f907fe0
p = subprocess.Popen(['./times-up-again'],stdout=-1,stdin=-1,stderr=-1)
p.stdin.write(bytes(str(eval(p.stdout.readline()[10:]) & 18446744073709551615,'utf-8') + b'\n')
#                            receive the challenge; chop off 'Challenge: '
#                       evaluate the challenge and mod by 1 << 64
#                       note (1<<64)-1 = 18446744073709551615, so we bitwise &
#             and send the answer through
p.stdin.flush()
for i in range(4): print(p.stdout.readline())
```
After running a few times (it works somewhat reliably), we are greeted with the flag.
> ``picoCTF{Hasten. Hurry. Ferrociously Speedy. #16462951}``
