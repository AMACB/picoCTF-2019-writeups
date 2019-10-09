# pastaAAA
> This pasta is up to no good. There MUST be something behind it.

After downloading the provided image file, there isn't much for us to do. As this is a CTF forensics problem, the easiest first check would be least significant bit steganography, so we whip out the Python Image Library.
```python3
>>> from PIL import Image
>>> def extractLSB(mask):
...     im = Image.open('ctf.png')
...     im_lsbs = im.point(lambda x : 255 // mask * (x&mask))
...     im_lsbs.save('lsb_'+bin(mask)+'.png')
...
>>> for i in range(8):
...     extractLSB(1 << i)
```
Even though any individual bit does not give a clear image, each of the last 3 bits creates an image that looks like the picoCTF logo with a difficult-to-make-out flag in the top left corner. So instead of having these bits separated, we extract all three at once, and ``extractLSB(0b111)`` has the flag in the top-left corned of the image.
> ``picoCTF{pa$ta_1s_lyf3}``
