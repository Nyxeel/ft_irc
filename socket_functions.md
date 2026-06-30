# Socket & System Functions Reference

## Includes

```cpp
#include <sys/socket.h>   // socket, bind, connect, listen, accept, send, recv, setsockopt, getsockname
#include <netinet/in.h>   // htons, htonl, ntohs, ntohl, sockaddr_in
#include <arpa/inet.h>    // inet_addr, inet_ntoa, inet_ntop
#include <netdb.h>        // getprotobyname, gethostbyname, getaddrinfo, freeaddrinfo
#include <unistd.h>       // close, lseek
#include <fcntl.h>        // fcnprotoype bindtl
#include <sys/stat.h>     // fstat
#include <poll.h>         // poll
#include <signal.h>       // signal, sigaction, sigemptyset, sigfillset, sigaddset, sigdelset, sigismember
#include <cstdlib>        // atoi
```

---

## Socket erstellen & konfigurieren

### `socket`
```c
int socket(int domain, int type, int protocol);
```
Erstellt einen neuen Socket und gibt einen File Descriptor zurück.
- `domain`: `AF_INET` (IPv4), `AF_INET6` (IPv6)
- `type`: `SOCK_STREAM` (TCP), `SOCK_DGRAM` (UDP)
- `protocol`: meist `0` (automatisch)
- **Rückgabe**: fd >= 0 bei Erfolg, -1 bei Fehler
- **Einsatz**: Erster Schritt bei jedem Server/Client — Socket-FD ist Basis für alle weiteren Operationen.

---

### `close`
```c
int close(int fd);
```
Schließt einen File Descriptor (Socket oder Datei).
- **Einsatz**: Verbindung trennen, Ressourcen freigeben. Immer aufrufen wenn Socket nicht mehr gebraucht wird.

---

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
- **Einsatz**: Direkt nach `socket()`, vor `bind()`.

---

### `getsockname`
```c
int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
Gibt die lokale Adresse zurück, an die der Socket gebunden ist.
- **Einsatz**: Wenn Port dynamisch vergeben wurde (z.B. `bind()` mit Port 0) und man den tatsächlichen Port herausfinden will.

---

## Adressauflösung

### `getprotobyname`
```c
struct protoent *getprotobyname(const char *name);
```
Gibt Protokoll-Info anhand des Namens zurück (z.B. `"tcp"` → Protokollnummer 6).
```c
struct protoent *pe = getprotobyname("tcp");
```
- **Einsatz**: Selten nötig; wenn man die Protokollnummer explizit braucht statt `0` in `socket()`.

---

### `gethostbyname`
```c
struct hostent *gethostbyname(const char *name);
```
Löst einen Hostnamen in eine IP-Adresse auf (veraltet, aber noch verbreitet).
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
- **Einsatz**: Adressauflösung vor `bind()` (Server) oder `connect()` (Client).

---

### `freeaddrinfo`
```c
void freeaddrinfo(struct addrinfo *res);
```
Gibt die von `getaddrinfo` allozierte Liste wieder frei.
- **Einsatz**: Immer nach `getaddrinfo`, sobald die Adressen nicht mehr gebraucht werden.

---

## Verbindung aufbauen

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
- **Einsatz**: Server-Seite, nach `socket()` und `setsockopt()`.

---

### `connect`
```c
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
Verbindet einen Socket mit einer Remote-Adresse (Client-Seite).
- **Einsatz**: TCP-Client baut Verbindung zum Server auf.

---

### `listen`
```c
int listen(int sockfd, int backlog);
```
Versetzt einen gebundenen Socket in den Wartezustand für eingehende Verbindungen.
- `backlog`: Maximale Länge der Warteschlange (z.B. `SOMAXCONN` oder `128`)
- **Einsatz**: Server, nach `bind()`. Aktiviert den Socket für `accept()`.

---

### `accept`
```c
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```
Nimmt eine eingehende Verbindung an und gibt einen neuen Socket-FD zurück.
- `addr`: wird mit der Client-Adresse befüllt (kann `NULL` sein)
- **Rückgabe**: Neuer FD für diese spezifische Verbindung
- **Einsatz**: Server-Loop — für jeden Client ein eigener FD.

---

## Byte-Order Konvertierung

Netzwerk verwendet **Big Endian**, x86 verwendet **Little Endian** — diese Funktionen konvertieren.

### `htons` / `htonl`
```c
uint16_t htons(uint16_t hostshort);   // host to network, 16-bit (Port)
uint32_t htonl(uint32_t hostlong);    // host to network, 32-bit (IP)
```
Wandelt von Host-Byteorder → Netzwerk-Byteorder.
- **Einsatz**: Vor dem Setzen von Port und IP in `sockaddr_in`.

---

### `ntohs` / `ntohl`
```c
uint16_t ntohs(uint16_t netshort);    // network to host, 16-bit
uint32_t ntohl(uint32_t netlong);     // network to host, 32-bit
```
Wandelt von Netzwerk-Byteorder → Host-Byteorder.
- **Einsatz**: Nach dem Lesen von Port/IP aus empfangenen Paketen oder `sockaddr`.

---

## IP-Adressen Konvertierung

### `inet_addr`
```c
in_addr_t inet_addr(const char *cp);
```
Wandelt einen IPv4-String (`"192.168.1.1"`) in eine 32-bit Netzwerkadresse um.
- **Einsatz**: Veraltet — gibt bei Fehler `INADDR_NONE` zurück (problematisch). Bevorzuge `inet_pton`.

---

### `inet_ntoa`
```c
char *inet_ntoa(struct in_addr in);
```
Wandelt eine `in_addr` Struktur in einen IPv4-String zurück.
- **Einsatz**: IP-Adresse eines Clients als lesbaren String ausgeben. Veraltet (nicht thread-safe).

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
- **Einsatz**: IP-Adresse aus `sockaddr` in lesbaren String konvertieren.

---

## Daten senden & empfangen

### `send`
```c
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```
Sendet Daten über einen verbundenen Socket.
- `flags`: meist `0`, oder `MSG_NOSIGNAL` (verhindert SIGPIPE bei geschlossener Verbindung)
- **Rückgabe**: Anzahl gesendeter Bytes (kann < `len` sein!)
- **Einsatz**: Nachrichten an Client/Server schicken. Rückgabewert prüfen & ggf. wiederholen.

---

### `recv`
```c
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```
Empfängt Daten von einem verbundenen Socket.
- **Rückgabe**: Bytes gelesen; `0` = Verbindung geschlossen; `-1` = Fehler
- `flags`: meist `0`, oder `MSG_PEEK` (liest ohne zu konsumieren)
- **Einsatz**: Daten vom Client lesen im Server-Loop.

---

## Signalbehandlung

### `signal`
```c
void (*signal(int signum, void (*handler)(int)))(int);
```
Registriert einen einfachen Signal-Handler.
```c
signal(SIGINT, handler);
signal(SIGPIPE, SIG_IGN);  // Pipe-Fehler ignorieren
```
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
Verwalten eine Menge von Signalen (`sigset_t`), die z.B. in `sigaction.sa_mask` verwendet wird — Signale die während des Handlers blockiert werden sollen.

---

## Datei-Operationen

### `lseek`
```c
off_t lseek(int fd, off_t offset, int whence);
```
Setzt den Lese-/Schreibzeiger eines File Descriptors.
- `whence`: `SEEK_SET` (absolut), `SEEK_CUR` (relativ), `SEEK_END` (vom Ende)
- **Einsatz**: Dateigröße ermitteln: `lseek(fd, 0, SEEK_END)`.

---

### `fstat`
```c
int fstat(int fd, struct stat *buf);
```
Gibt Metadaten (Größe, Typ, Rechte, Timestamps) eines offenen FDs zurück.
- **Einsatz**: Dateigröße prüfen, Typ prüfen (Datei vs. Verzeichnis).

---

### `fcntl`
```c
int fcntl(int fd, int cmd, ... /* arg */ );
```
Verschiedene Steuerfunktionen für File Descriptors.
- **Non-blocking setzen** (wichtig für IRC-Server):
```c
fcntl(fd, F_SETFL, O_NONBLOCK);
```
- **Einsatz**: Sockets auf non-blocking setzen, damit `accept`/`recv` nicht blockieren.

---

## Multiplexing / Event Loop

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
- **Einsatz**: Herzstück des IRC-Servers — ein Thread verwaltet alle Clients ohne zu blockieren.

**Alternativen**: `select` (ältere API, limitiert auf FD < 1024), `epoll` (Linux, skalierbarer für viele Clients).

---

## Typischer Server-Ablauf

```
socket()
  └─ setsockopt()    (SO_REUSEADDR)
      └─ bind()
          └─ listen()
              └─ poll() / select()
                  ├─ accept()     ← neue Verbindung
                  └─ recv()       ← Daten von bestehendem Client
                      └─ send()   ← Antwort
```
