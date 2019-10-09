# AES-ABC
> AES-ECB is bad, so I rolled my own cipher block chaining mechanism - Addition Block Chaining! You can find the source here: aes-abc.py. The AES-ABC flag is body.enc.ppm

> Hints:
> You probably want to figure out what the flag looks like in ECB form...

As is always the case, we begin by skimming the source code. Of interest, of course, is the encryption algorithm.
```python3
def aes_abc_encrypt(pt):
    cipher = AES.new(KEY, AES.MODE_ECB)
    ct = cipher.encrypt(pad(pt))

    blocks = [ct[i * BLOCK_SIZE:(i+1) * BLOCK_SIZE] for i in range(len(ct) / BLOCK_SIZE)]
    iv = os.urandom(16)
    blocks.insert(0, iv)
    
    for i in range(len(blocks) - 1):
        prev_blk = int(blocks[i].encode('hex'), 16)
        curr_blk = int(blocks[i+1].encode('hex'), 16)

        n_curr_blk = (prev_blk + curr_blk) % UMAX
        blocks[i+1] = to_bytes(n_curr_blk)

    ct_abc = "".join(blocks)
 
    return iv, ct_abc, ct
```
The top states that the encryption mode is electronic codebook (ECB), and the hint suggests that we would like to get to reverse the ciphertext into this form.

Skimming down, it uses an initialization vector ``iv`` to start off the blocks, and then it runs the following critical chain:
```python
    for i in range(len(blocks) - 1):
        prev_blk = int(blocks[i].encode('hex'), 16)
        curr_blk = int(blocks[i+1].encode('hex'), 16)

        n_curr_blk = (prev_blk + curr_blk) % UMAX
        blocks[i+1] = to_bytes(n_curr_blk)
```
Essentially, it is taking the previous block and adding it directly to the current block, and then iterating this process with the next block. Note the accompanying ASCII diagram.
```
       [plain 1]            [plain 2]            [plain 3]       ...
           |                    |                    |
           | (AES ECB)          | (AES ECB)          | (AES ECB)
           V                    V                    V
       [ ECB 1 ]            [ ECB 1 ]            [ ECB 3 ]
+iv -> [ enc 1 ]   --- + -> [ enc 2 ]   --- + -> [ enc 3 ]   --- ...
```
Experienced readers may note this is pretty much the typical cipher block chaining, using addition instead of xor. We are, of course, given the ``enc`` blocks in the ``body.ppm.enc`` file, and while we can't reverse this into the original plaintext, we can at least reverse this back into ECB mode by simply subtracting out. Notice that
```
enc n = ECB n  +  enc n-1
```
where ``enc 0 = iv`` by convention. This means that we simply have
```
ECB n = enc n  -  enc n-1
```
which can be calculated easily.

We do this now. The script will output in ECB.
```python3
>>> import binascii
>>> f = open('body.enc.ppm','rb').read()[16:] # Remove the header
>>> enc_blocks = [f[i:i+16] for i in range(0,len(f),16)]
>>> enc_blocks = [int(binascii.hexlify(b),16) for b in enc_blocks]
>>> ecb_blocks = [(enc_blocks[n] - enc_blocks[n-1]) % (256**16) for n in range(1,len(enc_blocks))]
```

This will, theoretically, turn our ciphertext into the simpler ECB mode, so now we must exploit the ECB. The issue with ECB is that the same plaintext becomes the same ciphertext every time, so in cases where many plaintexts will inevitably repeated, we could guess what the plaintext corresponds to brcause it appears so frequently. Or, because it is an image being encrypted (it is ``.ppm``), we could hope that each block corresponds to one red-green-blue triple so that the same pixel will get encrypted to the same block in ciphertext, every single time.

And this wishful thinking pays off. Indeed, if one runs ``set(ecb_blocks)``, there are surprisingly few total values (in comparison to the modulus ``UMAX``), so it is likey that these rgb values correspond. Skimming through ``ecb_blocks``, it is easily noticed that the value ``170711812579352817628051274266082219019`` appears far more than any other value, so we guess that this corresponds to a background color of some kind. Because of how little the other values seem to appear, we can will just pretend that this value is white and all other values are black. (This simply makes the image crisper and makes image-creation easier.)

So we create the following image-creation function.
```python3
>>> from PIL import Image
>>> bkgd = 170711812579352817628051274266082219019
>>> def create(w, h):
...     im = Image.new('RGB', (w,h))
...     im.putdata([(0,0,0) if b==bkgd else (255,255,255) for b in ecb_blocks])
...     im.save('flags/flag' + str(w) + str('.png'))
```
I couldn't get the image dimensions given in the header of the ``.ppm`` to work for me, but some for loop magic does the trick. After some guessing and checking (``for i in range(1,1000):create(i,len(ecb_blocks)//i+i)``), we see the flag is especially visible for widths of length ``355`` and multiples. 
> ``picoCTF{d0Nt_r0ll_yoUr_0wN_aES}``
