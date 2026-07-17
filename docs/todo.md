# ft_irc — TODO

## Fortschritt

```
[✓] Socket Setup        [✓] poll() Loop         [✓] accept() / disconnect
[ ] Buffer Handling     [ ] Send Functions      [ ] Command Parser
[ ] Auth Commands       [ ] Channel Commands    [ ] Operator Commands
```

---

## 1 — Infrastruktur

| Status | Task | Beschreibung |
|--------|------|--------------|
| `[✓]` | **Buffer-Handling** | Partielle Messages mit `\r\n` als Trennzeichen sammeln — pro Client ein `_buffer` |
| `[ ]` | **sendToClient()** | Nachricht an einen einzelnen Client schicken |
| `[ ]` | **sendToAll()** | Nachricht an alle Members eines Channels schicken (außer Sender) |
| `[ ]` | **handleCommand()** | Rohe Message parsen — Kommando und Argumente trennen |

---

## 2 — Authentication Commands

> Müssen in dieser Reihenfolge kommen. Kein anderes Kommando erlaubt bis alle drei erfolgreich.

| Status | Command | Verhalten |
|--------|---------|-----------|
| `[ ]` | **PASS** | Passwort prüfen → `_passOk = true` oder `464 :Password incorrect` |
| `[ ]` | **NICK** | Nickname setzen → auf Duplikate prüfen → `433 :Nickname in use` |
| `[ ]` | **USER** | Username setzen → alle drei ok? → Welcome `001` senden |

---

## 3 — Channel Commands

| Status | Command | Verhalten |
|--------|---------|-----------|
| `[ ]` | **JOIN** | Channel erstellen oder beitreten → alle Members informieren |
| `[ ]` | **PRIVMSG** | Nachricht an Client weiterleiten |
| `[ ]` | **PART** | Channel verlassen → alle Members informieren |
| `[ ]` | **QUIT** | Server verlassen → alle Channels informieren → disconnect |

---

## 4 — Operator Commands

> Nur Channel Operators dürfen diese Kommandos ausführen.

| Status | Command | Verhalten |
|--------|---------|-----------|
| `[ ]` | **KICK** | Client aus Channel entfernen |
| `[ ]` | **INVITE** | Client in Channel einladen |
| `[ ]` | **TOPIC** | Channel Topic anzeigen oder ändern |
| `[ ]` | **MODE +i** | Invite-only Channel setzen/entfernen |
| `[ ]` | **MODE +t** | TOPIC auf Operators beschränken |
| `[ ]` | **MODE +k** | Channel Passwort setzen/entfernen |
| `[ ]` | **MODE +o** | Operator-Rechte vergeben/entziehen |
| `[ ]` | **MODE +l** | User-Limit setzen/entfernen |

---

## IRC Numeric Replies Cheatsheet

```
001  RPL_WELCOME          :Welcome to the IRC Network <nick>
433  ERR_NICKNAMEINUSE    <nick> :Nickname is already in use
464  ERR_PASSWDMISMATCH   :Password incorrect
461  ERR_NEEDMOREPARAMS   <cmd> :Not enough parameters
403  ERR_NOSUCHCHANNEL    <channel> :No such channel
482  ERR_CHANOPRIVSNEEDED <channel> :You're not channel operator
```
