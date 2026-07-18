/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/18 16:24:44 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include <arpa/inet.h> // htons(), inet_ntop()
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>  // fcntl(), O_NONBLOCK
#include <iostream> // close()
#include <poll.h>   // poll(), struct pollfd
#include <signal.h>
#include <stdexcept>
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <unistd.h>     // close()
#include <vector>

// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────

Server::Server(std::string port, std::string password) {

	_serverSocket = -1;
	char *endptr;
	long tmpPort = strtol(port.c_str(), &endptr, 10);
	if (*endptr != '\0' || tmpPort <= 1024 || tmpPort > 65535)
	    throw std::runtime_error("Port: invalid");
	_port = static_cast<uint16_t>(tmpPort);
  	_password = password;
  	_running = false;

}

Server::Server(const Server &other)
    : _serverSocket(other._serverSocket), _port(other._port),
      _password(other._password), _running(other._running) {}

Server &Server::operator=(const Server &other) {

  if (this != &other) {

    _serverSocket = other._serverSocket;
    _port = other._port;
    _password = other._password;
    _running = other._running;
  }
  return *this;
}

Server::~Server() { cleanSockets(); }

// ───────────────────────────────────────────────
// ─────────────────── SIGNALS ───────────────────
// ───────────────────────────────────────────────

static void signalHandler(int sig) {

	(void)sig;
	if (g_server)
		g_server->stop();
}

void Server::init_signals() {
  struct sigaction sa;

  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGINT, &sa, NULL);
  signal(SIGPIPE, SIG_IGN);
}

void printServerStop() {

	struct timespec ts = {0, 300000000L}; // 300ms
	for (int i = 0; i < 10; i++) {
	  const char *dots[] = {"   ", ".  ", ".. ", "..."};
	  std::cout << "\rServer shutting down" << dots[i % 4] << std::flush;
	  nanosleep(&ts, NULL);
	}
	std::cout << "\r                        \r" << std::flush;
}

void Server::stop() {

  if (!_running)
    return;
  _running = false;
}

inline void Server::cleanSockets() {

	if (_serverSocket != -1) {
	  close(_serverSocket);
	  _serverSocket = -1;
	}
	printServerStop();
}

// ───────────────────────────────────────────────
// ──────────────────── SETUP ────────────────────
// ───────────────────────────────────────────────

void Server::setup() {

	init_signals();

	// Socket erstellen
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == FATAL)
	  throw std::runtime_error(std::string("Error socket(): ") +
	                           strerror(errno));

							   // Socket konfigurieren (SO_REUSEADDR)
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
	    -1)
	  throw std::runtime_error(std::string("Error setsockopt(): ") +
	                           strerror(errno));

							   // Non-blocking setzen
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) == FATAL)
	  throw std::runtime_error(std::string("Error fcntl(): ") + strerror(errno));

	  // Port/IP in Netzwerk-Byteorder umwandeln
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//	An Port binden
	if (bind(_serverSocket, (struct sockaddr *)&_addr, sizeof(_addr)) == FATAL)
	  throw std::runtime_error(std::string("Error bind(): ") + strerror(errno));

	  // Auf Verbindungen warten
	if (listen(_serverSocket, SOMAXCONN) == FATAL)
	  throw std::runtime_error(std::string("Error listen(): ") + strerror(errno));

	_running = true;
}

// ───────────────────────────────────────────────
// ─────────────────── POLL LOOP ─────────────────
// ───────────────────────────────────────────────

void Server::run() {

	std::vector<pollfd> fds;

	struct pollfd serverfd;         // ein pollfd-Eintrag für den Server-Socket
	serverfd.fd = _serverSocket;    // welcher fd überwacht werden soll
	serverfd.events = POLLIN;       // worauf gewartet wird: "lesbar" = neue Verbindung wartet


	fds.push_back(serverfd);        // in die Liste aller überwachten fds aufnehmen

	while(_running) {

		int polls = poll(fds.data(), fds.size(), -1);
		if (polls == 0)
			throw std::runtime_error("Error poll: system call timed out"); //redundant -> poll(timeout = -1)
		if (polls < 0) {
			if (!_running)
        		break;  // Signal interrupt with control + c
			throw std::runtime_error(std::string("Error poll: ") + strerror(errno));
		}

		std::vector<pollfd> addClients;
		for (iterator socket = fds.begin(); socket != fds.end(); socket++) {

			if (!(socket->revents & POLLIN) && !(socket->revents & POLLHUP))
    			continue;  // skip wenn KEINE Events

			if (socket->fd == _serverSocket){

  				struct sockaddr_in clientAddr;
  				socklen_t clientSize = sizeof(clientAddr);
  				memset(&clientAddr, 0, clientSize);

  				int clientSocket = accept(_serverSocket, (sockaddr *)&clientAddr, &clientSize);
  				if (clientSocket == FATAL)
  				  throw std::runtime_error(std::string("Error accept(): ") +
  				                           strerror(errno));

				// Client Non-blocking setzen
  				if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == FATAL)
  				  throw std::runtime_error(std::string("Error fcntl(): ") + strerror(errno));

  				_clientMap[clientSocket] =	Client(clientSocket);


				struct pollfd clientfd;
				clientfd.fd = clientSocket;
				clientfd.events = POLLIN;

				addClients.push_back(clientfd); //adde clients nach dem for loop um foor loop size nicht zu veraendern

				char ip[INET_ADDRSTRLEN];
				if (inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip)) == NULL)
				    std::cout << "New connection (unknown ip)" << std::endl;
				else
				    std::cout << "New connection from " << ip << ":" << ntohs(clientAddr.sin_port) << std::endl;
			}



			else {
				// Ereignis auf CLIENT
				//reciv() and send()

				char buffer[4096];
  				memset(buffer, 0, sizeof(buffer));
  				int bytesReceived = recv(socket->fd, buffer, sizeof(buffer) - 1, 0);
				if (bytesReceived <= 0) {

					if (bytesReceived == 0)
						std::cout << "Client disconnected" << std::endl;
					else
						std::cerr << "recv failed: " << strerror(errno) << std::endl;

					close(socket->fd);           		// Server schließt FD
					_clientMap.erase(socket->fd);  		// Client destructor wird gecalled und closed _clientfd
					_parser.clearClient(socket->fd);	// loescht den eintrag in der map fuer IRCMessage
					socket = fds.erase(socket);
					socket--;              				// Schleife macht danach socket++, das gleicht das aus
					continue;
				}


				std::vector<IrcMessage> msgs = _parser.processBuffer(socket->fd, std::string(buffer, bytesReceived));
				for (size_t i = 0; i < msgs.size(); i++)
				    handleCommand(socket->fd, msgs[i]);

			}
		}

		for (size_t i = 0; i < addClients.size(); i++)
    			fds.push_back(addClients[i]);
	}
}

// ───────────────────────────────────────────────
// ──────────────── SEND / REPLY ─────────────────
// ───────────────────────────────────────────────

void Server::handleCommand(int fd, const IrcMessage& msg) {



	if (msg.command == "PASS")
		handlePass(fd, msg);
	else if (msg.command == "NICK")
		handleNick(fd, msg);
	else if (msg.command == "USER")
		handleUser(fd, msg);
	else if (!_clientMap[fd].isAuthenticated()) {
        // JOIN, PRIVMSG etc. vor Login → Fehler
        sendToClient(fd, ":ircserv " + std::string(ERR_NOTREGISTERED) + " * :You have not registered\r\n");
        return;
  	}
	else if (msg.command == "JOIN")
		; // handleJoin(fd, msg);
	else if (msg.command == "PRIVMSG")
		; // handlePrivmsg(fd, msg);
	else
	    sendToClient(fd, ":ircserv " + std::string(ERR_UNKNOWNCOMMAND) + " " + _clientMap[fd].getNickname() + " " + msg.command + " :Unknown command\r\n");
}

void	Server::sendToClient(int fd, const std::string& msg) {


	size_t 	totalSent = 0;
	int 	bytesSent;

	while (totalSent < msg.size()) {

		bytesSent = send(fd, msg.c_str() + totalSent, msg.size() - totalSent, 0);
		if (bytesSent <= 0) {

			if  (bytesSent == FATAL)
				std::cerr << "Send failed: " << strerror(errno) << std::endl;
			break;
		}
		totalSent += bytesSent;
	}
}

// ───────────────────────────────────────────────
// ──────────────── REGISTRATION ─────────────────
// ───────────────────────────────────────────────

void	Server::handlePass(int fd, const IrcMessage& msg) {

	if (_clientMap[fd].isAuthenticated() || _clientMap[fd].isPassOK()) {
		sendToClient(fd, ":ircserv " + std::string(ERR_ALREADYREGISTERED) + " * :You may not reregister\r\n");
		return ;
	}
	else if (msg.params.empty())
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " * PASS :Not enough parameters\r\n");
	else if (msg.params[0] == _password)
		_clientMap[fd].setPassOK();
	else
		sendToClient(fd, ":ircserv " + std::string(ERR_PASSWDMISMATCH) + " * :Password incorrect\r\n");

	if (_clientMap[fd].isPassOK() && _clientMap[fd].isNickOK() && _clientMap[fd].isUserOK() )
		sendWelcome(fd);

}

void	Server::handleNick(int fd, const IrcMessage& msg) {

	std::string target = _clientMap[fd].isNickOK() ? _clientMap[fd].getNickname() : "*";
	if (msg.params.empty()) {

		sendToClient(fd, ":ircserv " + std::string(ERR_NONICKNAMEGIVEN) + " " + target  + " :No nickname given\r\n");
		return ;
	}
	else if (_clientMap[fd].isNickOK() && msg.params[0] == _clientMap[fd].getNickname()) //eigener nick kein fehler
    	return;
	else if (!_parser.isValidNickname(msg.params[0])) {

		sendToClient(fd, ":ircserv " + std::string(ERR_ERRONEUSNICKNAME) + " " + target + " " + msg.params[0] + " :Erroneous nickname\r\n");
		return ;

	}
	else if (_parser.isNicknameInUse(msg.params[0], _clientMap)) {

		sendToClient(fd, ":ircserv " + std::string(ERR_NICKNAMEINUSE) + " " + target  + " " + msg.params[0] + " :Nickname is already in use\r\n");
		return ;
	}


	if (_clientMap[fd].isAuthenticated()) {

		// 	TODO: inform all other clients in channel about nickchange
		//	sendtoAllClients in channel
		// 	format :oldnick!user@host NICK :newnick

		sendToClient(fd, ":" + _clientMap[fd].getNickname() + "!" + _clientMap[fd].getUsername() + "@host NICK :" + msg.params[0] + "\r\n");
		_clientMap[fd].setNickname(msg.params[0]);
		return ;
	}
	_clientMap[fd].setNickname(msg.params[0]);
	_clientMap[fd].setNickOK();

	if (_clientMap[fd].isPassOK() && _clientMap[fd].isNickOK() && _clientMap[fd].isUserOK() )
		sendWelcome(fd);

}

void	Server::handleUser(int fd, const IrcMessage& msg) {

	if (_clientMap[fd].isAuthenticated() || _clientMap[fd].isUserOK()) {
    	sendToClient(fd, ":ircserv " + std::string(ERR_ALREADYREGISTERED) + " * :You may not reregister\r\n");
   		return;
	}
	if (msg.params.size() < 4) {

		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " * USER :Not enough parameters\r\n");
		return ;
	}

	_clientMap[fd].setUsername(msg.params[0]);
	_clientMap[fd].setUserOK();

	if (_clientMap[fd].isPassOK() && _clientMap[fd].isNickOK() && _clientMap[fd].isUserOK() )
		sendWelcome(fd);
}

void	Server::sendWelcome(int fd) {

		_clientMap[fd].setAuthenticate(); // Authenticate Client!
		std::string nick = _clientMap[fd].getNickname();
		std::string user = _clientMap[fd].getUsername();
    	sendToClient(fd, ":ircserv " + std::string(RPL_WELCOME) + " " + nick + " :Welcome to the IRC Network " + nick + "!" + user +"@127.0.0.1\r\n");

}

// ───────────────────────────────────────────────
// ──────────────── CHANNEL COMMANDS ─────────────
// ───────────────────────────────────────────────
// handleJoin, handlePart, handlePrivmsg, handleQuit


// ───────────────────────────────────────────────
// ──────────────── OPERATOR COMMANDS ────────────
// ───────────────────────────────────────────────
// handleKick, handleInvite, handleTopic, handleMode
