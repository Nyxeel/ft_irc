# 📖 Parsing-Modul (`ft_irc`) – Integrationsanleitung

Dieses Modul übernimmt das komplette **syntaktische Parsing** (TCP-Buffering, Zerlegen von IRC-Nachrichten) und **semantische Parsing** (Formatprüfungen, Duplikats-Validierung).

Alle Funktionen sind **C++98-konform** implementiert. Ausgaben erfolgen strikt über `true` / `false` bzw. sauber aufbereitete Datenstrukturen, damit die Steuerung der IRC-Error-Numerics vollständig in der Hand der Server-Logik bleibt.

---

## 1. Das Datenmodell (`IrcMessage`)

Der Parser zerlegt jede vollständige Zeile in diese Struktur. Du musst keine Strings manuell splitten:

```cpp
struct IrcMessage {
    std::string prefix;               // Optionaler Server-/User-Prefix (meist leer)
    std::string command;              // Der IRC-Befehl (z.B. "KICK", "NICK", "JOIN")
    std::vector<std::string> params;  // Argumente-Liste (inkl. Text nach dem ':')
};
```

*Hinweis zum Trailing Parameter:* Der Parser erkennt das IRC-Trennzeichen `:` automatisch. Der Text am Ende landet als **ein einzelnes Element** im Vektor, selbst wenn er Leerzeichen enthält (wichtig für `PRIVMSG` oder `USER`).

---

## 2. Integration in den Server-Loop (TCP-Buffering)

Da TCP Daten oft in unvollständigen Fragmenten sendet (z.B. `NIV` -> `MSG\n`), verwaltet der Parser interne Puffer pro Client-FD. Übergib einfach die rohen Bytes aus `recv()` direkt an den Parser:

```cpp
// 1. Instanziiere den Parser einmalig im Server
Parser parser;

// 2. Im Server-Loop, wenn Aktivität auf einem Client-Socket (poll/select) gemeldet wird:
char buffer[1024];
int bytesReceived = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

if (bytesReceived > 0) {
    buffer[bytesReceived] = '\0';
    
    // Der Parser setzt Fragmente zusammen und liefert eine Liste fertiger Befehle
    std::vector<IrcMessage> messages = parser.processBuffer(clientFd, buffer);
    
    // Verarbeite alle vollständig extrahierten Befehle nacheinander
    for (size_t i = 0; i < messages.size(); ++i) {
        executeCommand(clientFd, messages[i]); // Deine Routing-Funktion
    }
} 
else if (bytesReceived == 0) {
    // WICHTIG: Wenn der Client trennt, lösche seinen Puffer aus der Map!
    parser.clearClient(clientFd);
    close(clientFd);
}
```

---

## 3. Semantische Validierung (Hilfsfunktionen)

Die Klasse stellt statische Methoden bereit, mit denen du Eingaben prüfen und entsprechende IRC-Error-Codes (Numerics) triggern kannst.

### A) Nickname-Prüfung (`NICK`)
Prüft die RFC-Konformität (Länge, erlaubte Zeichen) und sucht in deiner Client-Map nach Duplikaten. Die Prüfung ist **case-insensitive** und berücksichtigt IRC-Sonderzeichen (laut RFC gilt `[` als Großbuchstabe von `{`).

```cpp
void handleNickCommand(int fd, const IrcMessage& msg, std::map<int, Client>& allClients) {
    if (msg.params.empty()) {
        // -> Sende ERR_NONICKNAMEGIVEN (431)
        return;
    }
    
    std::string chosenNick = msg.params[0];

    if (!Parser::isValidNickname(chosenNick)) {
        // -> Sende ERR_ERRONEUSNICKNAME (432)
    } 
    else if (Parser::isNicknameInUse(chosenNick, allClients)) {
        // -> Sende ERR_NICKNAMEINUSE (433)
    } 
    else {
        // Validierung erfolgreich, Nickname kann zugewiesen werden
        allClients[fd].nickname = chosenNick;
    }
}
```

### B) Username-Prüfung (`USER`)
Prüft die RFC-Konformität (Länge, erlaubte Zeichen) und sucht in deiner Client-Map nach Duplikaten. Die Prüfung ist **case-insensitive** und berücksichtigt IRC-Sonderzeichen (laut RFC gilt `[` als Großbuchstabe von `{`).

```cpp
void handleUserCommand(int fd, const IrcMessage& msg, std::map<int, Client>& allClients) {
    if (msg.params.empty()) {
        // -> Sende ERR_NOUSERNAMEGIVEN (431)
        return;
    }
    
    std::string chosenNick = msg.params[0];

    if (!Parser::isValidUsername(chosenUser)) {
        // -> Sende ERR_ERRONEUSUSERNAME (432)
    } 
    else if (Parser::isUsernameInUse(chosenUser, allClients)) {
        // -> Sende ERR_USERNAMEINUSE (433)
    } 
    else {
        // Validierung erfolgreich, Username kann zugewiesen werden
        allClients[fd].username = chosenUser;
    }
}
```

### C) Komma-Listen splitten (`JOIN` / `PRIVMSG`)
IRC-Clients senden Kanäle oder User oft als kommagetrennte Liste (z.B. `JOIN #chan1,#chan2`). Die Funktion teilt diese auf und ignoriert leere Segmente (z.B. `,,`).

```cpp
void handleJoinCommand(int fd, const IrcMessage& msg) {
    if (msg.params.empty()) {
        // -> Sende ERR_NEEDMOREPARAMS (461)
        return;
    }

    // Splittet "chan1,chan2" automatisch in einen std::vector<std::string>
    std::vector<std::string> channels = Parser::splitByComma(msg.params[0]);

    for (size_t i = 0; i < channels.size(); ++i) {
        if (!Parser::isValidChannelName(channels[i])) {
            // -> Sende ERR_NOSUCHCHANNEL (403) für dieses spezifische Element
        } 
        else {
            // Füge den Client dem Channel 'channels[i]' hinzu
        }
    }
}
```

---

## 4. API Übersicht (Methoden-Signaturen)

```cpp
// Instanz-Methoden (für das Buffering)
std::vector<IrcMessage> processBuffer(int clientFd, const std::string& rawData);
void                    clearClient(int clientFd);

// Statische Hilfsmethoden (für die Validierung)
static std::vector<std::string> splitByComma(const std::string& str);
static bool                     isValidNickname(const std::string& nick);
static bool                     isValidUsername(const std::string& user);
static bool                     isValidChannelName(const std::string& channel);
static bool                     isNicknameInUse(const std::string& nickname, const std::map<int, Client>& clients);
static bool                     isUsernameInUse(const std::string& username, const std::map<int, Client>& clients);
static bool                     ircStringCompare(const std::string& s1, const std::string& s2);
```
