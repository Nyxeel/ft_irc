# Projektübersicht

Das Ziel von **ft_irc** ist die Entwicklung eines IRC-Servers in **C++98**, der mehrere Clients gleichzeitig verwalten kann und vollständig auf nicht-blockierenden I/O-Operationen basiert.

Der Server muss unter anderem folgende Funktionen unterstützen:

* Authentifizierung
* Nicknames und Usernames
* Channels
* Private Nachrichten
* Channel-Operatoren
* IRC-Modi

---

# Entwicklungsphasen

---

# Phase 0 – Recherche & Vorbereitung

## Ziele

* IRC-Protokoll verstehen
* TCP/IP-Grundlagen lernen
* Nicht-blockierende Sockets verstehen
* poll() beherrschen

## Zu lernende Themen

### Netzwerkgrundlagen

* TCP vs. UDP
* IP-Adressen
* Ports
* Client-Server-Modell

### Socket-Programmierung

* `socket()`
* `bind()`
* `listen()`
* `accept()`
* `send()`
* `recv()`
* `close()`

### Event-Handling

* `poll()`
* File Descriptors
* Nicht-blockierende I/O

### IRC-Protokoll

Folgende Befehle verstehen:

* PASS
* NICK
* USER
* JOIN
* PRIVMSG
* KICK
* INVITE
* TOPIC
* MODE

---

# Phase 1 – Minimaler TCP-Server

## Ziel

Einen Server erstellen, der TCP-Verbindungen akzeptieren kann.

## Aufgaben

* [✅] Klasse `Server` erstellen
* [❌] Programmargumente parsen
* [❌] Listening-Socket erzeugen
* [❌] Socket konfigurieren
* [❌] Socket an Port binden
* [❌] Verbindungen akzeptieren
* [❌] Sauberes Beenden implementieren

## Test

Server starten:

```bash
./ircserv 6667 password
```

Verbindung herstellen:

```bash
nc localhost 6667
```

Erwartetes Ergebnis:

* Verbindung wird akzeptiert
* Server bleibt stabil

---

# Phase 2 – Event Loop mit poll()

## Ziel

Mehrere Clients gleichzeitig unterstützen.

## Aufgaben

* [❌] poll()-Loop erstellen
* [❌] pollfd-Liste verwalten
* [❌] Neue Verbindungen erkennen
* [❌] Eingehende Daten erkennen
* [❌] Verbindungsabbrüche behandeln

## Test

Mehrere Verbindungen gleichzeitig öffnen:

```bash
nc localhost 6667
```

Erwartetes Ergebnis:

* Mehrere Clients gleichzeitig verbunden
* Keine Blockierung des Servers

---

# Phase 3 – Client-Verwaltung

## Ziel

Jeden verbundenen Benutzer durch ein Objekt repräsentieren.

## Aufgaben

* [❌] Klasse `Client` erstellen
* [❌] File Descriptor speichern
* [❌] Nickname speichern
* [❌] Username speichern
* [❌] Authentifizierungsstatus speichern
* [❌] Empfangsbuffer speichern

## Beispielattribute

```cpp
int fd;
std::string nickname;
std::string username;
std::string buffer;
bool authenticated;
```

---

# Phase 4 – Daten empfangen

## Ziel

Nachrichten von Clients empfangen.

## Aufgaben

* [❌] recv() implementieren
* [❌] Verbindungsabbrüche erkennen
* [❌] Daten in den Buffer schreiben
* [❌] Debug-Ausgabe der empfangenen Daten

## Test

Mit netcat verbinden:

```bash
nc localhost 6667
```

Beliebige Nachrichten senden.

Erwartetes Ergebnis:

* Server empfängt alle Nachrichten korrekt

---

# Phase 5 – IRC-Parser

## Ziel

Rohdaten in IRC-Befehle umwandeln.

## Aufgaben

* [❌] Parser erstellen
* [❌] Kommando und Argumente trennen
* [❌] CRLF behandeln
* [❌] Unvollständige Pakete korrekt zusammensetzen

## Beispiel

Eingabe:

```text
JOIN #42
```

Ausgabe:

```text
Befehl: JOIN
Argument: #42
```

---

# Phase 6 – Authentifizierung

## Ziel

Clients korrekt registrieren.

## Zu implementierende Befehle

### PASS

```text
PASS password
```

### NICK

```text
NICK max
```

### USER

```text
USER max 0 * :Max Musermann
```

## Aufgaben

* [❌] Passwort prüfen
* [❌] Nicknames validieren
* [❌] Doppelte Nicknames verhindern
* [❌] Registrierung abschließen

## Test

Ein Benutzer gilt als registriert nach:

* PASS
* NICK
* USER

---

# Phase 7 – Verbindung mit einem echten IRC-Client

## Ziel

Verbindung über einen IRC-Client herstellen.

## Empfohlener Client

* HexChat

## Aufgaben

* [❌] Verbindung testen
* [❌] Registrierung testen
* [❌] Nickname-Handling testen

## Erfolgreich wenn

* HexChat sich ohne Fehler verbinden kann

---

# Phase 8 – Channel-System

## Ziel

Channels erstellen und verwalten.

## Aufgaben

* [❌] Klasse `Channel` erstellen
* [❌] Mitglieder speichern
* [❌] Operatoren speichern
* [❌] Topic speichern
* [❌] Channels dynamisch erzeugen

## Zu implementierender Befehl

### JOIN

```text
JOIN #42
```

## Test

Mehrere Benutzer treten demselben Channel bei.

---

# Phase 9 – Nachrichtensystem

## Ziel

Kommunikation zwischen Benutzern ermöglichen.

## Zu implementierender Befehl

### PRIVMSG

```text
PRIVMSG #42 :Hallo Welt
```

## Aufgaben

* [❌] Benutzer-zu-Benutzer Nachrichten
* [❌] Benutzer-zu-Channel Nachrichten
* [❌] Nachrichten an alle Channel-Mitglieder weiterleiten

## Test

Nachrichten erscheinen bei allen Teilnehmern.

---

# Phase 10 – Operator-Funktionen

## Ziel

Alle vom Subject geforderten Operator-Befehle implementieren.

---

## KICK

```text
KICK #42 user
```

Aufgaben:

* [❌] Benutzer entfernen
* [❌] Channel informieren

---

## INVITE

```text
INVITE user #42
```

Aufgaben:

* [❌] Einladungen verwalten
* [❌] Invite-Only Channels unterstützen

---

## TOPIC

```text
TOPIC #42 :Neues Thema
```

Aufgaben:

* [❌] Topic setzen
* [❌] Topic anzeigen

---

## MODE

### Invite Only

```text
MODE #42 +i
```

### Topic nur für Operatoren

```text
MODE #42 +t
```

### Passwortschutz

```text
MODE #42 +k geheim
```

### Operator vergeben

```text
MODE #42 +o user
```

### Benutzerlimit

```text
MODE #42 +l 10
```

Aufgaben:

* [❌] Alle Pflicht-Modi implementieren
* [❌] Rechte prüfen
* [❌] Fehlerfälle behandeln

---

# Phase 11 – Stabilität & Fehlerbehandlung

## Ziel

Einen robusten Server entwickeln.

## Aufgaben

* [❌] Verbindungsabbrüche behandeln
* [❌] Ungültige Befehle behandeln
* [❌] Doppelte Nicknames behandeln
* [❌] Ungültige Channels behandeln
* [❌] Speicher korrekt freigeben
* [❌] Alle Fehlerfälle testen

## Belastungstests

* [❌] Mehrere Clients gleichzeitig
* [❌] Schnelle Verbindungsabbrüche
* [❌] Fragmentierte Pakete
* [❌] Ungültige Eingaben

---

# Phase 12 – Vorbereitung auf die Evaluation

## Ziel

Projekt für die Verteidigung vorbereiten.

## Aufgaben

* [❌] Alle Pflichtanforderungen prüfen
* [❌] Debug-Ausgaben entfernen
* [❌] Testfälle dokumentieren
* [❌] README fertigstellen
* [❌] Code aufräumen

---

# Bonus

Erst beginnen, wenn der Mandatory Part vollständig und stabil funktioniert.

## Mögliche Erweiterungen

* [❌] IRC-Bot
* [❌] Dateiübertragung

---

# Fortschritt

## Server

* [❌] TCP-Server
* [❌] poll()
* [❌] Client-Verwaltung
* [❌] IRC-Parser

## Authentifizierung

* [❌] PASS
* [❌] NICK
* [❌] USER

## Channels

* [❌] JOIN
* [❌] Channel-Erstellung
* [❌] Channel-Verwaltung

## Nachrichten

* [❌] PRIVMSG Benutzer
* [❌] PRIVMSG Channel

## Operatoren

* [❌] KICK
* [❌] INVITE
* [❌] TOPIC
* [❌] MODE +i
* [❌] MODE +t
* [❌] MODE +k
* [❌] MODE +o
* [❌] MODE +l

## Fertig

* [❌] Alle Pflichtanforderungen erfüllt
* [❌] Stabil unter Lasttests
* [❌] Dokumentation abgeschlossen
* [❌] Bereit für die Evaluation

```
```
