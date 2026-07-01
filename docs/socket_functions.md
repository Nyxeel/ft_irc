# Socket & System Functions Reference

## Includes

```cpp
#include <sys/socket.h>   // socket, bind, connect, listen, accept, send, recv, setsockopt, getsockname
#include <netinet/in.h>   // htons, htonl, ntohs, ntohl, sockaddr_in
#include <arpa/inet.h>    // inet_addr, inet_ntoa, inet_ntop
#include <netdb.h>        // getprotobyname, gethostbyname, getaddrinfo, freeaddrinfo
#include <unistd.h>       // close, lseek
#include <fcntl.h>        // fcntl
#include <sys/stat.h>     // fstat
#include <poll.h>         // poll
#include <signal.h>       // signal, sigaction, sigemptyset, sigfillset, sigaddset, sigdelset, sigismember
#include <cstdlib>        // atoi
```

---

## Typischer Server-Ablauf

```
[1] signal / sigaction          ← Signalbehandlung zuerst registrieren
[2] socket()                    ← Socket erstellen
[3] setsockopt()                ← Socket konfigurieren (SO_REUSEADDR)
[4] fcntl()                     ← Non-blocking setzen
[5] htons / htonl               ← Port/IP in Netzwerk-Byteorder umwandeln
[6] bind()                      ← An Port binden
[7] listen()                    ← Auf Verbindungen warten
────────── Server-Loop ──────────
[8] poll()                      ← Auf Ereignisse warten
    ├─ accept()                 ← Neue Verbindung annehmen
    │   └─ fcntl()              ← Neuen Client-FD auch non-blocking setzen
    ├─ recv()                   ← Daten empfangen
    └─ send()                   ← Antwort schicken
────────── Shutdown ─────────────
[9] close()                     ← Alle FDs schließen
```

---

## [1] Signalbehandlung

> Zuerst registrieren — bevor irgendein Socket oder FD geöffnet wird.
> So kann der Server sauber beendet werden (SIGINT = Ctrl+C) und SIGPIPE nicht abstürzen.

### `signal`
```c
void (*signal(int signum, void (*handler)(int)))(int);
```
Registriert einen einfachen Signal-Handler.
```c
signal(SIGINT, handler);
signal(SIGPIPE, SIG_IGN);  // Pipe-Fehler ignorieren
```
- **Erfolg**: Gibt den alten Handler zurück (`SIG_DFL` oder `SIG_IGN` oder vorherige Funktion)
- **Fehler**: Gibt `SIG_ERR` zurück, `errno` gesetzt
- **Einsatz**: Einfache Fälle. Bevorzuge `sigaction` für zuverlässigeres Verhalten.

---

### `sigaction`
```c
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```
Robusterer Weg um Signal-Handler zu setzen — portabler und kontrollierbarer als `signal`.
```c
struct sigaction sa;
sa.sa_handler = handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;
sigaction(SIGINT, &sa, NULL);
```
- **Erfolg**: `0`
- **Fehler**: `-1`, `errno` gesetzt
- **Einsatz**: Bevorzugte Methode für Signalbehandlung im Server.

---

### Signal-Masken (`sigset_t`)

```c
int sigemptyset(sigset_t *set);          // Leere Menge
int sigfillset(sigset_t *set);           // Alle Signale
int sigaddset(sigset_t *set, int signo); // Signal hinzufügen
int sigdelset(sigset_t *set, int signo); // Signal entfernen
int sigismember(const sigset_t *set, int signo); // Prüfen ob enthalten
```
- **Erfolg**: `0` (alle außer `sigismember`); `sigismember` gibt `1` zurück wenn enthalten, `0` wenn nicht
- **Fehler**: `-1`, `errno` gesetzt

Verwalten eine Menge von Signalen (`sigset_t`), die z.B. in `sigaction.sa_mask` verwendet wird — Signale die während des Handlers blockiert werden sollen.

---

## [2] Socket erstellen

> Muss vor setsockopt, bind, listen kommen — alle diese Funktionen brauchen den FD den socket() zurückgibt.

### `socket`
```c
int socket(int domain, int type, int protocol);
```
Erstellt einen neuen Socket und gibt einen File Descriptor zurück.
- `domain`: `AF_INET` (IPv4), `AF_INET6` (IPv6)
- `type`: `SOCK_STREAM` (TCP), `SOCK_DGRAM` (UDP)
- `protocol`: meist `0` (automatisch)
- **Erfolg**: File Descriptor (`>= 0`)
- **Fehler**: `-1`, `errno` gesetzt

---

## [3] Socket konfigurieren

> Direkt nach socket(), vor bind() — sonst hat die Option keinen Effekt mehr.

### `setsockopt`
```c
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
```
Setzt Optionen auf einem Socket.
- Typische Verwendung: `SO_REUSEADDR` — verhindert "Address already in use" beim Neustart
```c
int opt = 1;
setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```
- **Erfolg**: `0`
- **Fehler**: `-1`, `errno` gesetzt

---

## [4] Non-blocking setzen

> Vor bind() setzen damit der Server-Socket von Anfang an non-blocking ist.
> Nach accept() erneut aufrufen — der neue Client-FD erbt den Modus nicht automatisch.

### `fcntl`
```c
int fcntl(int fd, int cmd, ... /* arg */ );
```
Verschiedene Steuerfunktionen für File Descriptors.
- **Non-blocking setzen** (wichtig für IRC-Server):
```c
fcntl(fd, F_SETFL, O_NONBLOCK);
```
- **Erfolg**: Abhängig von `cmd` — bei `F_SETFL` ist es `0`
- **Fehler**: `-1`, `errno` gesetzt
- **Einsatz**: Sockets auf non-blocking setzen, damit `accept`/`recv` nicht blockieren.

---

## [5] Byte-Order Konvertierung

> Vor bind() — Port und IP müssen in Netzwerk-Byteorder stehen wenn sie in sockaddr_in geschrieben werden.

Netzwerk verwendet **Big Endian**, x86 verwendet **Little Endian** — diese Funktionen konvertieren.

### `htons` / `htonl`
```c
uint16_t htons(uint16_t hostshort);   // host to network, 16-bit (Port)
uint32_t htonl(uint32_t hostlong);    // host to network, 32-bit (IP)
```
Wandelt von Host-Byteorder → Netzwerk-Byteorder.
- **Erfolg**: Konvertierter Wert (kein Fehlerfall — diese Funktionen können nicht scheitern)
- **Einsatz**: Vor dem Setzen von Port und IP in `sockaddr_in`.

### `ntohs` / `ntohl`
```c
uint16_t ntohs(uint16_t netshort);    // network to host, 16-bit
uint32_t ntohl(uint32_t netlong);     // network to host, 32-bit
```
Wandelt von Netzwerk-Byteorder → Host-Byteorder.
- **Erfolg**: Konvertierter Wert (kein Fehlerfall — diese Funktionen können nicht scheitern)
- **Einsatz**: Nach dem Lesen von Port/IP aus empfangenen Paketen oder `sockaddr`.

---

## IP-Adressen Konvertierung

> Wird beim Befüllen der sockaddr_in Struktur gebraucht (vor bind) oder zum Loggen einer Client-IP (nach accept).

### `inet_addr`
```c
in_addr_t inet_addr(const char *cp);
```
Wandelt einen IPv4-String (`"192.168.1.1"`) in eine 32-bit Netzwerkadresse um.
- **Erfolg**: 32-bit Adresse in Netzwerk-Byteorder
- **Fehler**: `INADDR_NONE` (`0xFFFFFFFF`) — problematisch weil das auch eine gültige Adresse ist
- **Einsatz**: Veraltet — bevorzuge `inet_pton`.

---

### `inet_ntoa`
```c
char *inet_ntoa(struct in_addr in);
```
Wandelt eine `in_addr` Struktur in einen IPv4-String zurück.
- **Erfolg**: Pointer auf statischen String (z.B. `"192.168.1.1"`)
- **Fehler**: kein definierter Fehlerfall
- **Einsatz**: IP-Adresse eines Clients nach accept() als lesbaren String ausgeben. Veraltet (nicht thread-safe).

---

### `inet_ntop`
```c
const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
```
Moderner Ersatz für `inet_ntoa` — unterstützt IPv4 und IPv6, thread-safe.
```c
char ip[INET_ADDRSTRLEN];
inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
```
- **Erfolg**: Pointer auf `dst` (den übergebenen Puffer)
- **Fehler**: `NULL`, `errno` gesetzt
- **Einsatz**: IP-Adresse aus `sockaddr` in lesbaren String konvertieren (nach accept).

---

## Adressauflösung

> Optional — vor socket() oder bind() wenn man Hostnamen statt IPs verwenden will.

### `getprotobyname`
```c
struct protoent *getprotobyname(const char *name);
```
Gibt Protokoll-Info anhand des Namens zurück (z.B. `"tcp"` → Protokollnummer 6).
```c
struct protoent *pe = getprotobyname("tcp");
```
- **Erfolg**: Pointer auf `protoent` Struktur
- **Fehler**: `NULL`
- **Einsatz**: Selten nötig; wenn man die Protokollnummer explizit braucht statt `0` in `socket()`.

---

### `gethostbyname`
```c
struct hostent *gethostbyname(const char *name);
```
Löst einen Hostnamen in eine IP-Adresse auf (veraltet, aber noch verbreitet).
- **Erfolg**: Pointer auf `hostent` Struktur
- **Fehler**: `NULL`, `h_errno` gesetzt (nicht `errno`!)
- **Einsatz**: Hostname → IP für Client-Verbindungen. Bevorzuge `getaddrinfo` in neuem Code.

---

### `getaddrinfo`
```c
int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints,
                struct addrinfo **res);
```
Moderner Ersatz für `gethostbyname` — IPv4 und IPv6 kompatibel.
- `node`: Hostname oder IP-String (oder `NULL` für localhost)
- `service`: Port als String oder Servicename (`"6667"`, `"http"`)
- `hints`: Filter (z.B. nur TCP, nur IPv4)
- `res`: Ergebnisliste (muss mit `freeaddrinfo` freigegeben werden)
- **Erfolg**: `0`
- **Fehler**: Fehlercode (nicht `-1`!) — lesbarer String mit `gai_strerror(ret)`
- **Einsatz**: Adressauflösung vor `bind()` (Server) oder `connect()` (Client).

---

### `freeaddrinfo`
```c
void freeaddrinfo(struct addrinfo *res);
```
Gibt die von `getaddrinfo` allozierte Liste wieder frei.
- **Rückgabe**: keiner (`void`) — kann nicht scheitern
- **Einsatz**: Direkt nach getaddrinfo, sobald die Adressen nicht mehr gebraucht werden — sonst Memory Leak.

---

## [6] An Port binden

> Nach socket(), setsockopt() und fcntl() — braucht den konfigurierten FD und die befüllte sockaddr_in Struktur.

### `bind`
```c
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
Bindet einen Socket an eine lokale Adresse und Port.
```c
struct sockaddr_in addr;
addr.sin_family = AF_INET;
addr.sin_port = htons(6667);
addr.sin_addr.s_addr = INADDR_ANY;
bind(fd, (struct sockaddr*)&addr, sizeof(addr));
```
- **Erfolg**: `0`
- **Fehler**: `-1`, `errno` gesetzt (häufig `EADDRINUSE` wenn Port bereits belegt)

---

### `getsockname`
```c
int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
Gibt die lokale Adresse zurück, an die der Socket gebunden ist.
- **Erfolg**: `0`, `addr` wird befüllt
- **Fehler**: `-1`, `errno` gesetzt
- **Einsatz**: Nach bind() — wenn Port dynamisch vergeben wurde (bind mit Port 0) und man den tatsächlichen Port herausfinden will.

---

## [7] Auf Verbindungen warten

> Nach bind(), vor dem Server-Loop. Aktiviert den Socket für accept() — ohne listen() ignoriert der Kernel eingehende Verbindungen.

### `listen`
```c
int listen(int sockfd, int backlog);
```
Versetzt einen gebundenen Socket in den Wartezustand für eingehende Verbindungen.
- `backlog`: Maximale Länge der Warteschlange (z.B. `SOMAXCONN` oder `128`)
- **Erfolg**: `0`
- **Fehler**: `-1`, `errno` gesetzt

---

## [8] Server-Loop

> Herzstück des Servers. poll() blockiert bis ein Ereignis auftritt — dann wird entschieden ob neue Verbindung (accept) oder Daten (recv/send).

### `poll`
```c
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```
Überwacht mehrere File Descriptors gleichzeitig auf Ereignisse.
```c
struct pollfd fds[MAX_CLIENTS];
fds[0].fd = server_fd;
fds[0].events = POLLIN;

int ret = poll(fds, nfds, -1); // -1 = blockiert bis Ereignis
if (fds[0].revents & POLLIN) { /* eingehende Verbindung */ }
```
- `POLLIN`: Daten lesbar
- `POLLOUT`: Schreiben möglich
- `POLLERR` / `POLLHUP`: Fehler / Verbindung getrennt
- **Erfolg**: Anzahl der FDs mit Ereignissen (`> 0`); `0` wenn Timeout abgelaufen ohne Ereignis
- **Fehler**: `-1`, `errno` gesetzt (z.B. `EINTR` wenn durch Signal unterbrochen)

**Alternativen**: `select` (ältere API, limitiert auf FD < 1024), `epoll` (Linux, skalierbarer für viele Clients).

---

### `accept`
```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
Nimmt eine eingehende Verbindung an und gibt einen neuen Socket-FD zurück.
- `addr`: wird mit der Client-Adresse befüllt (kann `NULL` sein)
- **Erfolg**: Neuer File Descriptor (`>= 0`) für diese spezifische Client-Verbindung
- **Fehler**: `-1`, `errno` gesetzt
- **Einsatz**: Nur aufrufen wenn poll() POLLIN auf dem Server-FD meldet. Danach fcntl() auf dem neuen FD aufrufen.

---

### `recv`
```c
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```
Empfängt Daten von einem verbundenen Socket.
- `flags`: meist `0`, oder `MSG_PEEK` (liest ohne zu konsumieren)
- **Erfolg**: Anzahl gelesener Bytes (`> 0`)
- **Verbindung geschlossen**: `0` → Client hat Verbindung getrennt, FD schließen
- **Fehler**: `-1`, `errno` gesetzt (bei non-blocking: `EAGAIN`/`EWOULDBLOCK` = keine Daten verfügbar, kein echter Fehler)
- **Einsatz**: Nur aufrufen wenn poll() POLLIN auf dem Client-FD meldet.

---

### `send`
```c
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```
Sendet Daten über einen verbundenen Socket.
- `flags`: meist `0`, oder `MSG_NOSIGNAL` (verhindert SIGPIPE bei geschlossener Verbindung)
- **Erfolg**: Anzahl gesendeter Bytes (`> 0`) — kann kleiner als `len` sein, dann Rest nochmal senden!
- **Fehler**: `-1`, `errno` gesetzt
- **Einsatz**: Nach recv() wenn eine Antwort nötig ist, oder wenn poll() POLLOUT meldet.

---

## [9] Shutdown / Verbindung trennen

> Aufrufen wenn Client sich trennt (recv gibt 0 zurück) oder beim Beenden des Servers.
> Alle offenen FDs schließen — sonst Resource Leak.

### `close`
```c
int close(int fd);
```
Schließt einen File Descriptor (Socket oder Datei).
- **Erfolg**: `0`
- **Fehler**: `-1`, `errno` gesetzt
- **Einsatz**: Client-FD schließen wenn recv() 0 zurückgibt. Server-FD schließen beim Beenden.

---

## Nur Client-Seite (nicht dein Code)

> connect() wird vom IRC-Client (WeeChat etc.) aufgerufen — du implementierst das nicht.

### `connect`
```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
Verbindet einen Socket mit einer Remote-Adresse (Client-Seite).
- **Erfolg**: `0`
- **Fehler**: `-1`, `errno` gesetzt (z.B. `ECONNREFUSED` wenn Server nicht läuft)

---

## Datei-Operationen (Bonus: File Transfer)

> Nur relevant für den Bonus-Teil (File Transfer). Im Pflichtprogramm nicht nötig.

### `lseek`
```c
off_t lseek(int fd, off_t offset, int whence);
```
Setzt den Lese-/Schreibzeiger eines File Descriptors.
- `whence`: `SEEK_SET` (absolut), `SEEK_CUR` (relativ), `SEEK_END` (vom Ende)
- **Erfolg**: Neue Position des Zeigers (als `off_t`)
- **Fehler**: `(off_t)-1`, `errno` gesetzt
- **Einsatz**: Dateigröße ermitteln: `lseek(fd, 0, SEEK_END)`.

---

### `fstat`
```c
int fstat(int fd, struct stat *buf);
```
Gibt Metadaten (Größe, Typ, Rechte, Timestamps) eines offenen FDs zurück.
- **Erfolg**: `0`, `buf` wird befüllt
- **Fehler**: `-1`, `errno` gesetzt
- **Einsatz**: Dateigröße prüfen (`buf.st_size`), Typ prüfen (Datei vs. Verzeichnis).
