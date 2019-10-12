# john_pollard
> Sometimes RSA [certificates](cert) are breakable

> Hints: The flag is in the format picoCTF{p,q}. Try swapping p and q if it does not work.

This question is pretty much reserach. After looking around for long enough for a tool to read the RSA certificates, we found that [``openssl``](http://www.gtopia.org/blog/2010/02/der-vs-crt-vs-cer-vs-pem-certificates/) can do the job. Running the command
```
openssl x509 -in cert -text -noout
```
will spit out all of the data necessary to reconstruct the RSA cryptosystem. Because the hint says that the flag will simply be the factored modulus, we only care about the following lines.
```
            Public Key Algorithm: rsaEncryption
                Public-Key: (53 bit)
                Modulus: 4966306421059967 (0x11a4d45212b17f)
                Exponent: 65537 (0x10001)
```
A public key that's only fifty-three bits is laughably brute-forcable by [online calculators](https://www.alpertron.com.ar/ECM.HTM) and even thirty seconds of python scripting.
```python3
>>> n = 4966306421059967
>>> [(d, n//d) for d in range(1, int(sqrt(n))+1, 2) if n % d == 0]
[(1, 4966306421059967), (67867967, 73176001)]
```
Regardless of approach, we get a relatively painless flag.
> ``picoCTF{73176001,67867967}``
