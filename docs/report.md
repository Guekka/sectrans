# Macrosafe

The goal of macrosafe is to provide a safe wrapper around macrohard, a simple library using raw sockets to communicate.

## Macrohard study

Macrohard is a very simple library that exposes two precompiled shared libraries: `libserver.so` and `libclient.so`.

The exposed functions are extremely simple: `startserver`, `stopserver`, `sendmsg`, `getmsg`. And that's it. To understand better the library, we decompiled the two libraries using the Hex-Rays decompiler. The cleaned-up code is available in [the attached file](./macrohard.c).

It is pretty standard socket code. The server is listening on a port, and the client connects to it. The client sends
the size of the message, then the message itself. The server reads the size of the message, then the message itself. The
server then closes the connection.

This code is extremely simple, and it would be easy to rewrite the entire library in a safe way. However, the goal of
this project is to provide a safe wrapper around this library, so we will have to keep it and work around its limitations.

## Macrohard vulnerabilities

There are several obvious vulnerabilities in this code:

- The server does not limit the size of the message it reads. This can lead to a buffer overflow.

```
    read(new_socket, message_buffer, 5ULL);
    int message_size = atoi(message_buffer);
    read(new_socket, message_buffer, message_size);
```

At this point in the code, the server just read the size of the message. It then reads the message itself, without
checking if the size is valid. This can lead to a buffer overflow.

In theory, the client will always send a valid size, but a malicious client could send an invalid size.

- Similarly, right after, the server copies the message to the provided buffer, without checking if the size is valid.

```
    strcpy(buffer, message_buffer);
```

These vulnerabilities are particularly important, as they might allow an attacker to execute arbitrary code on the server.

And there is no way to fix these vulnerabilities without modifying the library. Unfortunately, this means these vulnerabilities will be present in the safe wrapper.

## Macrosafe

Macrosafe is written in modern C++. Its API is even simpler than the one of macrohard: the user can create a `Channel` (or an `EncryptedChannel`), and use `send_message` and `receive_message` to send and receive messages. The `EncryptedChannel` uses the `libhydrogen` library to encrypt the messages.

## Threats and solutions

Besides the vulnerabilities in macrohard, what could be the threats to the security of macrosafe?

There are a lot of possibilities. Let's focus on some ones.

### Man-in-the-middle

If an attacker can intercept the communication between the client and the server, they can read the messages, and even modify them. This is a very common attack, and it is the reason why TLS is so important.

The `EncryptedChannel` class uses `libhydrogen` to encrypt the messages. Its algorithm is based on NORX v3.0 AEAD. It is a modern algorithm, and it is considered secure.

### Access to unauthorized files from the client

The client is supposed to be able to download files from the server, but not any file: only the ones that were previously uploaded by the client. If an attacker can access any file on the server, it is a problem.

This problem is solved in SecTrans.

Also, access control could be improved: the client should only be able to access its own files, not the files of other clients.

This problem is not solved in SecTrans, as it would contradict the specifications.

### Authentication

Only authorized clients should be able to connect to the server. This can be implemented using a password, or a public/private key pair.

This problem is not solved in SecTrans, by lack of time. But the current architecture allows for easy implementation of authentication.

### Denial of service

An attacker could try to flood the server with requests, in order to prevent legitimate clients from accessing the server. This is a very common attack, and it is the reason why rate limiting is so important.

Since this service is not critical, no DDOS protection is implemented.

### Concurrent access

Since packets are only 1024 bytes, most messages have to be split accross several packets. If two clients are trying to send a message at the same time, the packets might be mixed up, and the message might be corrupted.

Worse, a malicious client could voluntarily send packets while another client is sending a message. Malicious content could then be inserted in the message. If the message contained a file, the file would be modified. This seems highly unlikely, but if the file is a program, it could be used to execute arbitrary code on the client.

Macrosafe has limited protection against this attack: each packet contains a sequence number, and the server checks that the packets are received in the correct order. But a sufficiently motivated attacker could still manage to replace a packet.

Moreover, the whole message is encrypted before splitting. This means that if a packet is replaced, the whole message will be corrupted, and the client will not be able to decrypt it.

### Malicious server

If an attacker managed to run a malicious server node, it could send malicious content to the client. This could be used to execute arbitrary code on the client.

We assume this is not possible, as the server is supposed to be run by a trusted entity.


