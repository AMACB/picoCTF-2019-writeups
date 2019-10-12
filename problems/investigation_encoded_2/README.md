# investigation_encoded_2

> We have recovered a [binary](mystery) and 1 file: [image01](output). See what you can make of it. Its also found in `/problems/investigation-encoded-2_1_a4cbd68ce835252282cea861a81110f8` on the shell server. NOTE: The flag is not in the normal picoCTF{XXX} format.

This problem is remarkably similar to [investigation_encoded_1](../problems/investigation_encoded_1), and we recommend reading that writeup first. Here's the translated python code:
```python
isValid = lambda c: (c == ' ') or ('a' <= c and c <= 'z') or ('A' <= c and c <= 'Z') or ('0' <= c and c <= '9')
def getValue(val):
	return (secret[val/8] >> (7 - val%8)) & 1
def save(c):
	global buffChar, remain
	buffChar |= c
	if remain == 0:
		remain = 7
		output.write(chr(buffChar))
		buffChar = 0
	else:
		buffChar *= 2
		remain -= 1
def encode():
	failed = False
	for c in flag:
		if not isValid(c):
			failed = True
			break
		c = ord(c.lower())
		if (c == ord(' ')):
			c = 0x85
		elif (ord('0') <= c) and (c <= ord('9')):
			c += 0x4b
		c -= 0x61
		if c != 36:
			c = (c+18) % 36
		for i in range(indexTable[c], indexTable[c+1]):
			save(getValue(i))
	if failed:
		print "Invalid Characters in flag.txt\n./output is corrupted"
		exit(1)
	while remain != 7:
		save(0)

flagfile = open("flag.txt","r")
flag = flagfile.read()
output = open("output","w")
global buffChar, remain
buffChar = 0
remain = 7
encode()
output.close()
print "I'm done, check ./output"
```
Compared with investigation_encoded_1, there are a few differences:

- There is a `login` function which will crash the program in the binary. For testing purposes, this can easily be bypassed in gdb by jumping over the call to `login`. This function has been omitted from the above python translation.
- Digits (`0` through `9`) are now valid flag characters. The mapping is now: `a -> 0, b -> 1, ... z -> 25, 0 -> 26, 1 -> 27, ... 9 -> 35, [space] -> 36`. Each value is additionally `rot18`'d.
- Instead of `matrix`, we have `indexTable`, for which even indices are the start of the range and odd indices are the end of the range (as opposed to length and start).

In the same manner as the previous problem, we find that each character corresponds to a sequence of bits:
```
a : 101011101110111000
b : 1010101110111000
c : 10111000
d : 10101010111000
e : 101010101000
f : 11101010101000
g : 1110111010101000
h : 111011101110101000
i : 111010101000
j : 11101011101000
k : 1110101000
l : 1010101000
m : 101000
n : 1011101110111000
o : 1010111000
p : 101010111000
q : 101110111000
r : 11101010111000
s : 1000
t : 10111010101000
u : 1011101110111011101000
v : 10111011101011101000
w : 1110101110111010111000
x : 111010111011101000
y : 11101110111010101000
z : 1110111010101110111000
0 : 1110101010111000
1 : 1110101110101110111000
2 : 10111010111010111000
3 : 111010101010111000
4 : 1011101011101000
5 : 101110101011101000
6 : 101011101110101000
7 : 1110101011101000
8 : 1110111011101110111000
9 : 10111011101110111000
  : 0000
```
Again, we write a [script](solve.py) to decode and get the flag:

> `t1m3f1i350000000000071ed4c08`