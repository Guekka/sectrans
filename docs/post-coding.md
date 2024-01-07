# Exploiting SecTrans

## Intercepting the communication

Assuming we have access to the network, we started Wireshark to intercept the packets. As expected, we were able to see the messages sent by the client and the server. We can easily decrypt the base64 and get the metadata for each packet, but that's all. The content of the message is encrypted and cannot be read.

## Reverse engineering

We did not try preventing reverse engineering, as it is impossible to protect a binary from reverse engineering. We could obfuscate the code, but it would only slow down the attacker.

Furthermore, trying to prevent reverse engineering would be [security by obscurity](https://en.wikipedia.org/wiki/Security_through_obscurity). Instead, we believe that the security of the system should not rely on the fact that the attacker does not know how the system works.

We still attempted to open the client binary with IDA 8.1. As expected with C++ code, the decompiled code is not very readable. We were able to roughly understand the code, but as expected, we did not find any vulnerability. The "string" view of IDA did not help either.

Assuming access to the server binary, we can find the hardcoded username and password. This is obviously a huge issue, but this code is only a prototype. With a bit more time, we would have used a proper database.

## Fuzzing the client

The file reading code is extremely simple and relies on the standard library `ifstream`. We still tried fuzzing with zzuf. As expected, the client handled the fuzzing very well.

## Fuzzing the server

With the following command, we can open a raw socket and send packets to the server:
```sh
exec 3<>/dev/tcp/127.0.0.1/12345
```
We can then send random data to the server:
```sh
cat /dev/urandom >&3
```
During my testing, the server crashed each time, with the following error:
```
Error: Invalid base64 encoded data
```
While a bit annoying, this is not a security issue. The "crash" is actually a controlled exit, as the server is able to catch the exception and exit gracefully. It would be easy to make the code resilient to this kind of attack, by catching the exception and ignoring it.

---

However, if we send malicious data, we can exploit the issue we found in libserver: a buffer overflow is possible, by sending a packet with a size greater than 1024 bytes. This issue should be fixed directly in libserver. Maybe some kind of stack canary could be useful.

## Conclusion

Apart from the one in libserver.so, we did not find any vulnerability.
