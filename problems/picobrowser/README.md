# picobrowser

> This website can be rendered only by **picobrowser**, go and catch the flag! ``https://2019shell1.picoctf.com/problem/12255/`` ([link](https://2019shell1.picoctf.com/problem/12255/)) or http://2019shell1.picoctf.com:12255

> Hints: You dont need to download a new web browser

Recalling [Secret Agent](https://github.com/shiltemann/CTF-writeups-public/blob/master/PicoCTF_2018/writeup.md#web-exploitation-200-secret-agent) from last year's competition, we feel like we can attempt a similar attack. While last time we were supposed to disguise ourselves as a Google use, this time we want to change the browser we are using. Connecting once with the website to ``/flag`` greets us with the following text.
> You're not picobrowser! Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36

Trying this on another browser (my Windows machine comes with Edge installed) yeilds the following.
> You're not picobrowser! Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.102 Safari/537.36 Edge/18.18362

As the challenge states, we're probably going to have to change something in that header to ``picobrowser``. Playing a pure extrapolation game, it looks like the browser name is appended to the end in pure plaintext. With this in mind, we try the stupidest things first and simply manually set ``User-Agent`` to ``picobrowser`` using python requests; this at least somewhat looks like the ``Chrome`` and ``Edge`` portions of the header (minus a forward slash perhaps), so this at least has some slim chance.
```python3
>>> import requests
>>> session = requests.Session()
>>> headers = {'User-Agent':'picobrowser'}
```
Now we make a get request using that header.
```python3
>>> response = session.get('https://2019shell1.picoctf.com/problem/12255/flag', headers=headers)
>>> page = response.text
>>> page[page.index('picoCTF{') : page.index('}')+1]
'picoCTF{p1c0_s3cr3t_ag3nt_bbe8a517}'
```
And so we somehow have gotten the flag.
> ``picoCTF{p1c0_s3cr3t_ag3nt_bbe8a517}``
