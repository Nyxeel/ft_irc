# ft_irc

*This project has been created as part of the 42 curriculum by pjelinek, bschwarz.*

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

[irssi](https://irssi.org/) is used as the reference client.

Connect directly from the shell:

```bash
irssi -c 127.0.0.1 -p <port> -w <password>
```

Or connect from within an already running irssi session:

```
/connect 127.0.0.1 <port> <password>
```

Once connected, register a nickname and username, then join a channel:

```
/nick <nickname>
/join #channel
```

## Resources

### IRC Protocol

This server implements the classic IRC client-server protocol as defined in RFC 2812
(the commands required by the subject — `PASS`, `NICK`, `USER`, `JOIN`, `PRIVMSG`,
`KICK`, `INVITE`, `TOPIC`, `MODE` — all come from this RFC). IRCv3 extensions
(capability negotiation, message tags, SASL, etc.) are out of scope for this project.

- RFC 2812 — Internet Relay Chat: Client Protocol (primary reference) — https://datatracker.ietf.org/doc/html/rfc2812
- RFC 1459 — Internet Relay Chat Protocol (original specification, historical background) — https://datatracker.ietf.org/doc/html/rfc1459
- Modern IRC Client Protocol (ircdocs) — supplementary/clarifying reference, not fully implemented (see note above) — https://modern.ircdocs.horse/

### Network Programming

- Beej's Guide to Network Programming — https://beej.us/guide/bgnet/
- `man 2 poll`
- `man 2 socket`
- `man 2 recv`
- `man 2 send`

### AI Usage

AI was used to look up IRC numeric reply codes and their expected message format, and to verify correct `poll()` event loop structure.

All generated content was reviewed and tested before use.
