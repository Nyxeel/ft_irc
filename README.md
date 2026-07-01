# ft_irc

*This project has been created as part of the 42 curriculum by pjelinek and bschwarz.*

## Description

`ft_irc` is an IRC server implemented in C++98. It allows multiple clients to connect simultaneously and communicate in real time via the IRC protocol over TCP/IP.

The server handles authentication, nicknames, usernames, channels, private messages, and channel operator commands (`KICK`, `INVITE`, `TOPIC`, `MODE`).

All I/O is non-blocking and multiplexed through a single `poll()` call — no forking, no threading.

## Instructions

### Compile

```bash
make
```

### Run

```bash
./ircserv <port> <password>
```

### Connect with an IRC client

Example with WeeChat:

```bash
/server add local 127.0.0.1/<port> -password=<password>
/connect local
```

## Resources

### IRC Protocol

- RFC 1459 — https://datatracker.ietf.org/doc/html/rfc1459
- RFC 2812 — https://datatracker.ietf.org/doc/html/rfc2812
- Modern IRC reference — https://modern.ircdocs.horse/

### Network Programming

- Beej's Guide to Network Programming — https://beej.us/guide/bgnet/
- `man 2 poll`
- `man 2 socket`
- `man 2 recv`
- `man 2 send`

## AI Usage

AI was used to look up IRC numeric reply codes and their expected message format, and to verify correct `poll()` event loop structure.

All generated content was reviewed and tested before use.
