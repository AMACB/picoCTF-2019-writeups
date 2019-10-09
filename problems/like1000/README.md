# like1000
> This .tar file got tarred alot. Also available at /problems/like1000_0_369bbdba2af17750ddf10cc415672f1c.

> Hints:
> Try and script this, it'll save you alot of time

Downloading, the file, we get find that the file is named ``1000.tar``. With nothing better to do, we extract the file there, and it turns into ``999.tar``. This does not bode well, so we script it. A quick google search reveals that this can be done in python.
```python3
import tarfile

for i in range(1000,0,-1):
    tarfile.open(str(i) + '.tar').extractall()
```
After waiting for python to extract all the files, we are left with a ``flag.png``, which upon opening contains the flag.
> picoCTF{l0t5_0f_TAR5}
