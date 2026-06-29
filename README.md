ft_irc — Internet Relay Chat Server
Description
ft_irc is a fully functional IRC server written in C++98. The server handles multiple clients simultaneously using non-blocking I/O with poll(). It implements the core IRC protocol — authentication, channels, private messaging, and channel operator commands — and is compatible with any standard IRC client.

The server is single-process and single-threaded. All I/O multiplexing is handled by a single poll() call.

Instructions
Compilation

make
Execution

./ircserv <port> <password>
Argument	Description
port	Port the server listens on (e.g. 6667)
password	Connection password required by IRC clients
Connecting with a client

# Example with WeeChat
/server add local 127.0.0.1/6667 -password=yourpassword
/connect local
Partial data test (nc)

nc -C 127.0.0.1 6667
# type: com^D man^D d\n
Supported commands
Command	Description
PASS	Authenticate with server password
NICK	Set nickname
USER	Set username
JOIN	Join a channel
PRIVMSG	Send private or channel message
KICK	Eject a user from a channel (operator)
INVITE	Invite a user to a channel (operator)
TOPIC	Set/view channel topic (operator)
MODE	Set channel modes: i t k o l
Resources
IRC Protocol
RFC 1459 — Internet Relay Chat Protocol
RFC 2812 — IRC: Client Protocol
Modern IRC Client Protocol
Socket Programming
Beej's Guide to Network Programming
man 7 socket, man 2 poll, man 2 recv, man 2 send
IRC Clients used for testing
WeeChat
irssi
AI Usage
AI was used for the following tasks during this project:

Generating boilerplate for socket setup and poll() event loop structure
Looking up IRC numeric reply codes and their expected format
Writing helper functions and reviewing edge cases in command parsing
All AI-generated content was reviewed, tested, and understood before being integrated into the project.
