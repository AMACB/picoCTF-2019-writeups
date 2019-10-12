# investigation_encoded_1

> We have recovered a [binary](mystery) and 1 file: [image01](output). See what you can make of it. Its also found in `/problems/investigation-encoded-1_5_c4e49003ef959a1a25e1a66ab0f30ad8` on the shell server. NOTE: The flag is not in the normal picoCTF{XXX} format.

With some experimenting, we find that this binary seems to take the contents of `flag.txt` and write some data into `output`. The data in `output` seems to be raw, and not actually an image. If we `xxd` the original `output`, we get:
```
00000000: 8e8e ba3b b8ea 23a8 ea3a a3ba e3ae 2eee  ...;..#..:......
00000010: 2eee 3a3a a2ba 3ba8                      ..::..;.
```
Let's now reverse the binary. Here's a summary of what the binary seems to do:
- `main` loads `flag.txt` and calls `encode`
- `encode` iterates through each character in the flag, gets some values from a mysterious `matrix` global variable, and calls `save(getValue())` several times
- `save` does some bitwise manipulation and will occasionally write to `./output`
- `getValue` accesses the `secret` global variable in some way, and returns a single bit
Let's now translate the whole thing into python:
```python
isValid = lambda c: (c == ' ') or ('a' <= c and c <= 'z') or ('A' <= c and c <= 'Z')
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
		c = c.lower()
		if (c == ' '):
			c = '{'
		c = ord(c) - 0x61
		for i in range(matrix[2*c + 1], matrix[2*c + 1] + matrix[2*c]):
			save(getValue(i))
	if failed:
		print "Error, I don't know why I crashed"
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
Now, we can see in more detail what's happening:

- `encode` iterates through each character in the flag. It converts each character into an integer `c` by mapping `a -> 0, b -> 1, ... z -> 25, [space] -> 26`. Then, for each `i` in `[matrix[2*c+1], matrix[2*c+1] + matrix[2*c])`, it calls `save(getValue(i))`. (In this way, the even indices of `matrix` correspond to the length, and the odd indices correspond to the start index.)
- `getValue` returns the `i % 8`th bit of `secret(i / 8)`.
- `save` manages a buffer of bits. When the buffer is not filled (`remain > 0`), it appends the inputted bit to the buffer. Once the buffer is filled with 8 bits (`remain = 0`), it outputs a byte into the file and clears the buffer.
- `encode` will flush the buffer in `save` with `0`'s at the end.

Thus, each character in the flag corresponds to certain sequence of bits whose size is not necessarily aligned with a byte. Based on the values of `matrix` and `secret` (see `data.txt`), we can find these sequences:
```
a : 10111000
b : 111010101000
c : 11101011101000
d : 1110101000
e : 1000
f : 101011101000
g : 111011101000
h : 1010101000
i : 101000
j : 1011101110111000
k : 111010111000
l : 101110101000
m : 1110111000
n : 11101000
o : 11101110111000
p : 10111011101000
q : 1110111010111000
r : 1011101000
s : 10101000
t : 111000
u : 1010111000
v : 101010111000
w : 101110111000
x : 11101010111000
y : 1110101110111000
z : 11101110101000
  : 0000
```
Now, we simply write a [script](solve.py) to decode and get the flag:

> `encodeddbqkjjnbfz`

Note that this problem is very similar to [investigation_encoded_2](/problems/investigation_encoded_2).