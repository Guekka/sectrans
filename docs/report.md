# Report

## Macrohard study

Macrohard is a very simple library that exposes two precompiled shared libraries: `libserver.so` and `libclient.so`.

The exposed functions are extremely simple: `startserver`, `stopserver`, `sendmsg`, `getmsg`. And that's it. To understand better the library, we decompiled the two libraries using the Hex-Rays decompiler. The cleaned-up code is available in [the attached file](./macrohard.c).

It is pretty standard socket code. The server is listening on a port, and the client connects to it. The client sends
the size of the message, then the message itself. The server reads the size of the message, then the message itself. The
server then closes the connection.

This code is extremely simple, and it would be easy to rewrite the entire library in a safe way. However, the goal of
this project is to provide a safe wrapper around this library, so we will have to keep it and work around its limitations.

## Macrohard vulnerabilities

There are several obvious vulnerabilities in macrohard:

- The server does not limit the size of the message it reads. This can lead to a buffer overflow.

```
    read(new_socket, message_buffer, 5ULL);
    int message_size = atoi(message_buffer);
    read(new_socket, message_buffer, message_size);
```

At this point in the code, the server just read the size of the message. It then reads the message itself, without checking if the size is valid. This can lead to a buffer overflow.

In theory, the client will always send a valid size, but a malicious client could send an invalid size.

- Similarly, right after, the server copies the message to the provided buffer, without checking if the size is valid.

```
    strcpy(buffer, message_buffer);
```

These vulnerabilities are particularly important, as they might allow an attacker to execute arbitrary code on the server.

And there is no way to fix these vulnerabilities without modifying the library. Unfortunately, this means these vulnerabilities will be present in the safe wrapper.

## Macrosafe

Macrosafe is a wrapper around macrohard, written in modern C++. Its API is even simpler than the one of macrohard: the user can create a `Channel` (or an `EncryptedChannel`), and use `send_message` and `receive_message` to send and receive messages. The `EncryptedChannel` uses the `libhydrogen` library to encrypt the messages.

Thanks to this simple API, it is almost impossible to misuse the library.

## Threats and solutions

Besides the vulnerabilities in macrohard, what could be the threats to the security of macrosafe?

There are a lot of possibilities. Let's focus on some ones.

### Man-in-the-middle

If an attacker can intercept the communication between the client and the server, they can read the messages, and even modify them. This is a very common attack, and it is the reason why TLS is so important.

The `EncryptedChannel` class uses `libhydrogen` to encrypt the messages. Its algorithm is based on NORX v3.0 AEAD. It is a modern algorithm, and it is considered secure.

### Access to unauthorized files

The client is supposed to be able to download files from the server, but not any file: only the ones that were previously uploaded by a client. If an attacker can access any file on the server, it is a problem.

This problem is solved in SecTrans.

It would also make sense to implement a per-file policy: some files could be public, and some files could be private. Since the specification requires the files to be accessible by any client, this is not implemented.

### Authentication

Only authorized clients should be able to connect to the server. This can be implemented using a password, or a public/private key pair.

SecTrans provides a basic authentication mecanism: the client provides its username and password in each request. The server checks that the username and password are correct before executing the request.

The username and password are sent in clear text, but the message is encrypted, so it is not a problem.

The password is hashed with libhydrogen's `hydro_pwhash` function.

For this prototype, the username and password are hardcoded in the server. Obviously, this is not secure, but it is only a prototype. With a bit more time, we would have used a proper database.

### Denial of service

An attacker could try to flood the server with requests, in order to prevent legitimate clients from accessing the server.

Since this service is not critical, no DDOS protection is implemented.

### Concurrent access

Since packets are only 1024 bytes, most messages have to be split accross several packets. If two clients are trying to send a message at the same time, the packets might be mixed up, and the message might be corrupted.

Worse, a malicious client could voluntarily send packets while another client is sending a message. Malicious content could then be inserted in the message. If the message contained a file, the file would be modified. This seems highly unlikely, but if the file is a program, it could be used to execute arbitrary code on the client.

Macrosafe has a relatively good protection against this attack: each packet contains a sequence number, and the server checks that the packets are received in the correct order. But a sufficiently motivated attacker could still manage to replace a packet.

Moreover, the whole message is encrypted before splitting. This means that if a packet is replaced, the whole message will be corrupted, and the client will not be able to decrypt it.

### Malicious server

If an attacker managed to run a malicious server node, it could send malicious content to the client. This could be used to execute arbitrary code on the client.

We assume this is not possible, as the server is supposed to be ran by a trusted entity.

## SecTrans overview

### High level

- The server is started and waits for a client to connect.
- The user can start the client with command line arguments. The user is prompted for a username and a password.
- The client connects to the server, and sends the username, the password, and the requested command.
- The server checks that the username and password are correct. If they are not, the server closes the connection.
- The server executes the requested command, and sends the result to the client.
- The client displays the result, and closes the connection.

### Low level

Messages have to be serialized to account for several constraints: messages are sent in packets of 1024 bytes, and they cannot contain null bytes.

The content of the message is first serialized to JSON with the well-known `nlohmann/json` library. The JSON is then serialized to a string, and the string is encrypted with `libhydrogen`. The encrypted string is then split into packets of ~700 bytes, which are then converted to base64. Each packet contains some metadata, including the sequence number, the total number of packets, and the size of the encrypted message.

The server receives the packets, and checks that they are received in the correct order. It then decrypts the message, and deserializes it from JSON.

## Development practices

In order to be secure, code has to follow best practices. Here are some of the practices we followed:
- Use modern C++: we used C++20, with no manual memory management. Standard containers and smart pointers are used everywhere.
- Use a modern build system: we used CMake, which is a modern build system.
- Use static analysis: we used clang-tidy to check for potential bugs. The compiler warnings are enabled, and treated as errors.
- Write tests: we used Catch2 to write unit tests. The coverage is not high enough but could be improved with more time.
- Separate concerns: the features are clearly separated, with message handling in a separate library (macrosafe). The code is also well-organized, with a clear separation between the client and the server.

## Conclusion

SecTrans is relatively secure, particularly compared to the original macrohard library. Future work could include:
- Implementing a per-file access policy.
- Implementing a more secure authentication mecanism with a real database or a public/private key pair.
- Replacing `libhydrogen` library. It is a popular library, but not as much as `libsodium` for example.
- Replacing `macrohard` with a more secure library. Some vulnerabilities will never be fixed otherwise.
