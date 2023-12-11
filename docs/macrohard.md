# Findings about Macrohard library

- It is used to send messages between a server and a client.
- It has a very restricted API: start, stop, send, receive.
- It does not seem possible to have more than one server per process.
- The library has an internal queue of 4 messages: if the queue is full, the send function blocks.
    - However, it looks like it unblocks after a few seconds. Some kind of timeout?

- The messages do not seem to be encrypted.
- There are no safety mechanisms.
