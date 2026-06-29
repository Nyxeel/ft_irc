# IRC-Server Lernguide

Themen, Ressourcen und Reihenfolge fuer die Implementierung eines IRC-Servers in C++ 98.

---

## Inhalt

1. [IRC-Protokoll](#irc-protokoll)
2. [TCP/IP Socket-Programmierung](#tcpip-socket-programmierung)
3. [Non-blocking I/O + poll()](#non-blocking-io--poll)
4. [IRC-Nachrichten-Parsing](#irc-nachrichten-parsing)
5. [IRC Commands](#irc-commands)
6. [C++ 98 Standard](#c-98-standard)
7. [Empfohlene Lernreihenfolge](#empfohlene-lernreihenfolge)

---

## IRC-Protokoll

**Was es ist:** IRC ist ein textbasiertes Netzwerkprotokoll. Nachrichten haben das Format:

```
[:prefix] COMMAND [params] [:<trailing>]\r\n
```

Du musst diese Nachrichten parsen und generieren.

**Ressourcen:**

- [RFC 1459 (Original)](https://datatracker.ietf.org/doc/html/rfc1459)
- [RFC 2812 (aktualisiert)](https://datatracker.ietf.org/doc/html/rfc2812)
- [Modern IRC (lesbar)](https://modern.ircdocs.horse/)

---

## TCP/IP Socket-Programmierung

**Was es ist:** Sockets sind Datei-Deskriptoren fuer Netzwerkverbindungen. Du oeffnest einen Socket, bindest ihn an einen Port, und akzeptierst eingehende Verbindungen.

**Ressourcen:**

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — der Standard-Leitfaden

Beej erklaert genau die Funktionen die das Subject listet: `socket`, `bind`, `listen`, `accept`, `send`, `recv`, `htons`, etc.

---

## Non-blocking I/O + poll()

**Was es ist:** Normalerweise blockiert `recv()` bis Daten ankommen. Mit `poll()` wartest du auf mehrere Sockets gleichzeitig und weisst vorher welcher bereit ist — so kannst du viele Clients mit einem Thread bedienen (kein `fork`).

**Ablauf:**

```
poll() → "welche fds haben Daten?" → nur dann read/write
```

**Ressourcen:**

- Terminal-Doku: `man poll` / `man select` / `man epoll_wait`
- [Beej's Guide, Kapitel "poll" / "select"](https://beej.us/guide/bgnet/html/#poll)

---

## IRC-Nachrichten-Parsing

**Was es ist:** Jede Zeile vom Client endet mit `\r\n`. TCP kann Pakete fragmentieren, also musst du einen Buffer pro Client fuehren und erst parsen wenn `\r\n` vollstaendig ankam (das ist der `nc`-Test im Subject).

**Beispiel:**

```
Client sendet: "JOI"  →  dann "N #room\r\n"
              →  du baust im Buffer zusammen bis \r\n
```

---

## IRC Commands

Diese Commands musst du implementieren:

| Command   | Zweck                  |
|-----------|------------------------|
| `PASS`    | Passwort-Auth          |
| `NICK`    | Nickname setzen        |
| `USER`    | Username / Realname    |
| `JOIN`    | Channel beitreten      |
| `PRIVMSG` | Nachricht senden       |
| `KICK`    | User aus Channel werfen|
| `INVITE`  | User einladen          |
| `TOPIC`   | Channel-Topic          |
| `MODE`    | Kanal-Modus aendern    |

Alle Antworten vom Server benutzen **numerische Codes** (z.B. `001` = Welcome, `433` = Nick in use).

→ Vollstaendige Liste: [modern.ircdocs.horse/numerics.html](https://modern.ircdocs.horse/numerics.html)

---

## C++ 98 Standard

**Was es ist:** Kein `auto`, kein range-for, kein `nullptr` — du verwendest `std::map`, `std::vector`, `std::string`, und klassische Iteratoren.

**Ressourcen:**

- [cppreference](https://en.cppreference.com/w/) (C++98-kompatibel filtern)

---

## Empfohlene Lernreihenfolge

1. **Beej's Guide lesen** — Sockets verstehen
2. **`poll()`-Beispiel bauen**, das 2 Clients gleichzeitig lesen kann
3. **RFC 2812 / Modern IRC lesen** — Message-Format + Handshake:
   ```
   PASS → NICK → USER → :server 001 ...
   ```
4. **Commands implementieren**
