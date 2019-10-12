# Java Script Kiddie

> The image link appears broken... https://2019shell1.picoctf.com/problem/37826 or http://2019shell1.picoctf.com:37826.
> Hints: This is only a JavaScript problem.

Upon accessing the site, we find a textbox with a submit button next to it. When we press submit, a placeholder image icon appears. We suspect that inputting a correct password into the box will make the image valid. Let's inspect the javascript, given that this is a JS-only problem by the hint:

```js
var bytes = [];
$.get("bytes", function(resp) {
	bytes = Array.from(resp.split(" "), x => Number(x));
});

function assemble_png(u_in){
	var LEN = 16;
	var key = "0000000000000000";
	var shifter;
	if(u_in.length == LEN){
		key = u_in;
	}
	var result = [];
	for(var i = 0; i < LEN; i++){
		shifter = key.charCodeAt(i) - 48;
		for(var j = 0; j < (bytes.length / LEN); j ++){
			result[(j * LEN) + i] = bytes[(((j + shifter) * LEN) % bytes.length) + i]
		}
	}
	while(result[result.length-1] == 0){
		result = result.slice(0,result.length-1);
	}
	document.getElementById("Area").src = "data:image/png;base64," + btoa(String.fromCharCode.apply(null, new Uint8Array(result)));
	return false;
}
```
We note the AJAX query for `bytes`. Accessing `https://2019shell1.picoctf.com/problem/37826/bytes` gives us a large array of [bytes](bytes.txt) represented as integers. We also read the code to see that `bytes` is treated as a 2D array with 16 columns; each column is shifted by the corresponding value in `key`. (For instance, if the third digit in `key` is `7`, then the third column is shifted down by `7`). We can write a python script to help visualize this:

```python
def tohex(a):
	return "{:02x}".format(a)

f = open("bytes.txt", "r")
bytelist = map(int, f.read().split(" "))
f.close()

key = "0000000000000000"
result = [0 for x in bytelist]
SIZE = 16
for i in range(SIZE):
	shift = ord(key[i]) - 48
	for j in range(len(result) / SIZE):
		result[(j*SIZE + i)] = bytelist[(((j+shift)*SIZE) % len(bytelist)) + i]

for i in range(len(result) / SIZE):
	print ' '.join(map(tohex, result[i*SIZE:(i+1)*SIZE]))
```
As output, we get:
```
35 48 9b 0e 0c 0a b7 0a f8 17 fd 0c fc 00 00 4e
1b 9f e1 74 82 00 72 72 e6 c0 c5 81 7f 48 44 00
17 4a ef cc 0d 49 00 41 f1 1d 00 21 17 c0 5f 52
44 97 93 1b 00 24 1a 1b f6 fe 00 54 00 4d 6e 6c
89 13 6f 60 4c 44 01 e5 00 23 00 8b 00 06 39 9c
00 4b fa 47 05 bd 44 20 00 31 00 00 49 43 e3 52
a4 8f 99 72 03 14 2f 88 01 e2 9c 00 00 ea 17 c8
40 6b 3c 02 1d ab 82 8e 54 00 c8 0d 9b f1 e2 30
ae ae 42 5f f8 5b 5a e4 c9 00 97 00 e0 cc ac df
c2 50 4e 99 33 7f 0e 3f 48 00 45 ed 75 bd 40 7b
e2 00 01 31 e6 00 fd 31 18 00 5e 51 6f a1 ec b2
e3 00 00 fc 9e be 92 1f 17 78 99 45 78 8c 37 a7
a2 10 85 0d cf b4 b4 e2 75 07 e5 49 cc a0 1d 6b
47 06 47 4f ce 04 64 1f 08 a0 e6 bc e2 08 60 01
1a 96 99 e4 7f 09 d9 aa e2 be ff 1f 18 e6 bf 02
00 33 36 cc 54 c4 3f 13 7d 2f f2 dd 7f b0 78 f8
f6 2d 5a 9c e5 c5 c2 98 f1 93 a7 a7 67 87 a6 fb
80 3f e2 90 9f 97 79 98 bf a0 af c5 1b e4 58 02
0f 45 92 06 ff 88 ff 24 bf 4f 6f 24 ae 7f f1 91
fc 18 cc 99 8e 1d 1f 82 61 5d 53 c4 04 c0 d2 07
f7 eb c2 c4 a8 1a 7d f4 a1 e7 b1 40 09 c7 af b7
1c 58 ec 3d a2 37 96 dd 0f c6 d5 2f 02 78 d9 ff
c6 e9 95 61 fc 55 e1 d5 e3 9b b4 f9 cd 23 1d 52
44 f1 83 82 f5 bb bf bd 5a bf b1 e0 18 92 f0 a4
0b 5c 7f f2 0d 4c 56 37 ba 43 07 5f a2 c5 e5 aa
fa d1 7b 61 85 f1 52 b6 84 3f 7c aa 88 59 ef 4e
6f 2b 02 e7 b7 ec a1 35 a4 08 79 c9 39 41 b7 31
bf 68 ab d2 7f d3 7e bb 7b ef 57 ec b7 82 77 72
33 1e d9 d6 79 ab 07 b5 d5 c8 7a 35 e9 d2 b7 26
bd a4 6f f9 19 c4 62 28 1f 9c cf 6d 95 5b 22 44
0c 16 ca cd 5b e0 e8 67 c9 34 8e dd 33 7c 9e 9f
7d 79 7c 42 34 27 14 e7 3e a9 b0 57 1f 9f 91 7c
3e e5 f2 33 af c1 f9 45 eb 80 a7 02 be be 7a 2d
d5 f1 a2 6e ed 9a 5b f3 31 05 ea 07 e9 ac 0c be
99 5e 5e 13 f5 23 4b 64 5a e8 2e 8e 99 96 1c bc
39 e5 e7 ea bc 2e ff 7f e9 04 1e 33 ef 49 57 17
d5 0b e3 4a f9 ee f2 e7 bd aa ff 6f af 86 00 da
e9 89 37 ca 7f 90 fe 64 fd 5d 5b 9f a5 f6 fe 00
db 5f 60 e1 bf 7d 94 78 ff a0 01 d3 02 c4 9f a4
17 92 74 37 21 b4 c6 95 be 5b 24 9e 3f 7f 02 c4
53 76 eb de f4 00 c0 00 fb 4d fe 97 8a 49 45 ff
```
If instead the key were to be `0070000000000000`, the third _column_ would begin with `3c, 42, 4e, 01`, etc.

From here, the key observation to make is that this data must represent a PNG image; we also know that a PNG must begin with the following header:
```
89 50 4e 47 0d 0a 1a 0a
```
Based on this alone, we can find that the first half of the key is `49952030`, which rotates the columns to get the header to be correct. Furthermore, we know that this header must be immediately followed by an `IHDR` chunk, which always has length `0d`. Thus, our first row must look like:
```
89 50 4e 47 0d 0a 1a 0a 00 00 00 0d 49 48 44 52
```
We can thus quickly find that the correct key must be of the form `49952030???75112`. The three `?`s are there because the corresponding byte (`00`) has multiple possible values (`4,5,6` for the first, `7,8,9` for the second (we can't have the two-digit `10`), and `2,3,4,5` for the third). We could brute force these possibilites, but we can reduce our search space by look at the second row. By the [spec](http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html), this corresponds to the content of the `IHDR` chunk, which must look like:
```
Width:              4 bytes
Height:             4 bytes
Bit depth:          1 byte
Color type:         1 byte
Compression method: 1 byte
Filter method:      1 byte
Interlace method:   1 byte
```
Let's see what the three `?`s must be:

- The first `?` must make the `bit depth` valid (1, 2, 4, 8, 16); the only way we can do this is having the first `? = 5`.
- The second `?` must make the `color type` valid (0, 3 based on the `bit depth`); we must have the second `? = 7,8,9`.
- The third `?` must make the `compression method` valid (0 is the only one allowed); we must have the third `? = 2,3,4`.

With only nine possibilites left, we can simply brute force the possible keys: `4995203057275112`, `4995203057375112`, `4995203057475112`, `4995203058275112`, `4995203058375112`, `4995203058475112`, `4995203059275112`, `4995203059375112`, `4995203059475112`. The last one gives us a QR code:
![QR Code](qr.png "QR Code")
The QR [decodes](https://zxing.org/w/decode.jspx) to the flag:

> `picoCTF{b7794daf95ade3c353aa8618c3a7e2c6}`