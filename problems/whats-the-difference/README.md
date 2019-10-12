# whats-the-difference

> Can you spot the difference? [kitters](kitters.jpg) [cattos](cattos.jpg). They are also available at /problems/whats-the-difference_0_00862749a2aeb45993f36cc9cf98a47a on the shell server

> Hints: How do you find the difference between two files? Dumping the data from a hex editor may make it easier to compare.

The title, challenge description, and hints all suggest that we want to check for where the files are different. We could use a hex editor, but python works fine.
```python3
>>> k = open('kitters.jpg','rb').read()
>>> c = open( 'cattos.jpg','rb').read()
```
At this point, we blindly run a byte-by-byte comparison of the two files.
```python3
>>> [(k[i], c[i]) for i in range(len(k)) if k[i] != c[i]]
[(153, 112), (157, 105), (152, 99), (200, 111), (10, 67), (244, 84), ...
```
Recalling that "p" evaluates to ``112`` with ASCII, we are inspired to attempt to extract the flag entirely from cattos, and we are thankfully granted the flag.
```python3
>>> ''.join(chr(c[i]) for i in range(len(k)) if k[i] != c[i])
'picoCTF{th3yr3_a5_d1ff3r3nt_4s_bu773r_4nd_j311y_aslkjfdsalkfslkflkjdsfdszmz10548}'
```
> ``picoCTF{th3yr3_a5_d1ff3r3nt_4s_bu773r_4nd_j311y_aslkjfdsalkfslkflkjdsfdszmz10548}``
