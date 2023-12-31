# SecTrans

## Description

SecTrans is an application made for a cybersecurity course. It is supposed to allow *sec*ure *trans*fer of files.

The main constraint was using the provided `libserver.so` and `libclient.so`. These two dynamic libraries are simple wrapper around sockets, with some limitations. See [the preliminery report](docs/report.md) for more details.

A [post-coding report](docs/post-coding.md) is also available.

**Do not use this code in production**. It is not secure at all. It is only a proof of concept, and I did not have the time to implement all the security features I wanted.

## Running

The user is expected to use `Nix` in order to get the dependencies. The Determinate Systems installer is recommanded:
```sh
curl --proto '=https' --tlsv1.2 -sSf -L https://install.determinate.systems/nix | sh -s -- install
```
Once Nix is installed, running `nix develop --impure` will put you in a working environment.

You can then build the project:
```sh
cmake --preset linux-gcc
cmake --build build
```

Finally, run the server:
```sh
./build/src/server/cybersec_project_server
```

The client (on the same machine) will then connect automatically. Available commands are:
- `-up <file>`: upload a file
- `-down <file>`: download a file and write it locally, overwriting existing content
- `-list`: list available files

The client binary is available at `build/src/client/cybersec_project_client`.

**Ports 12345 and 12346 are expected to be available**.
