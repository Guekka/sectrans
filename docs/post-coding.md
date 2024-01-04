# Exploiting SecTrans

## Introduction

This document describes our attempts to exploit SecTrans.

## Intercepting the communication

Assuming we have access to the network, we started Wireshark to intercept the packets. As expected, we were able to see the messages sent by the client and the server. We can easily decrypt the base64 and get the metadata for each packet, but that's all. The content of the message is encrypted and cannot be read.

## Reverse engineering

We did not try reverse engineering, as it is impossible to protect a binary from reverse engineering. We could obfuscate the code, but it would only slow down the attacker.

Furthermore, trying to prevent reverse engineering would be [security by obscurity](https://en.wikipedia.org/wiki/Security_through_obscurity). Instead, we believe that the security of the system should not rely on the fact that the attacker does not know how the system works.

## Fuzzing the client

// TODO

The file reading code is extremely simple and relies on the standard library `ifstream`. We still tried fuzzing with zzuf. We di

## Fuzzing the server

// TODO

## Conclusion

// TODO
