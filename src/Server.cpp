/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/21 09:24:28 by pjelinek         ###   ########.fr       */
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
#include <string>
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <unistd.h>     // close()
#include <vector>

void print(std::string str);

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
    			continue;  // skip rest wenn KEINE Events

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

				//poll struct
				struct pollfd clientfd;
				clientfd.fd = clientSocket;
				clientfd.events = POLLIN;

				addClients.push_back(clientfd); //adde clients nach dem for loop um foor loop size nicht zu veraendern

				char ip[INET_ADDRSTRLEN];
				if (inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip)) == NULL)
				    std::cout << "New connection (unknown ip)" << std::endl;
				else
				    std::cout << "New connection from " << ip << ":" << ntohs(clientAddr.sin_port) << std::endl;
				_clientMap[clientSocket].setHostAdresse(ip);
			}

			else {
				// Ereignis auf CLIENT
				//reciv() and send()

				char buffer[4];
  				memset(buffer, 0, sizeof(buffer));
  				int bytesReceived = recv(socket->fd, buffer, sizeof(buffer) - 1, 0);
				if (bytesReceived <= 0) {

					if (bytesReceived == 0)
						std::cout << "Client disconnected" << std::endl;
					else
						std::cerr << "recv failed: " << strerror(errno) << std::endl;

					broadcastQuit(socket->fd);			// sends quit message to all joined channels
					_clientMap.erase(socket->fd);  		// Client destructor wird gecalled und closed _clientfd
					_parser.clearClient(socket->fd);	// loescht den eintrag in der map fuer IRCMessage
					close(socket->fd);           		// Server schließt FD


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

void Server::handleCommand(int fd, const IrcMessage& msg) {


	if (msg.command == CMD_PASS)
		handlePass(fd, msg);
	else if (msg.command == CMD_NICK)
		handleNick(fd, msg);
	else if (msg.command == CMD_USER)
		handleUser(fd, msg);
	else if (!_clientMap[fd].isAuthenticated()) {
        // JOIN, PRIVMSG etc. vor Login → Fehler
        sendToClient(fd, ":ircserv " + std::string(ERR_NOTREGISTERED) +
			" * :You have not registered\r\n");
        return;
  	}
	else if (msg.command == CMD_JOIN)
		handleJoin(fd, msg);
	else if (msg.command == CMD_PRIVMSG)
		handlePrivmsg(fd, msg);
	else if (msg.command == CMD_KICK)
		handleKick(fd, msg);
	else if (msg.command == CMD_INVITE)
		handleInvite(fd, msg);
	else if (msg.command == CMD_TOPIC)
		handleTopic(fd, msg);
	else if (msg.command == CMD_MODE)
		; // handleMode(fd, msg);
	else
	    sendToClient(fd, ":ircserv " + std::string(ERR_UNKNOWNCOMMAND) + " "
			+ _clientMap[fd].getNickname() + " " + msg.command + " :Unknown command\r\n");
}

// ───────────────────────────────────────────────
// ──────────────── SEND / BROADCAST ─────────────
// ───────────────────────────────────────────────
// sendToClient, sendWelcome, sendChannelWelcome, broadcastToChannel

void	Server::sendToClient(int fd, const std::string& msg) {


	size_t 	totalSent = 0;
	int 	bytesSent;

	while (totalSent < msg.size()) {

		bytesSent = send(fd, msg.c_str() + totalSent, msg.size() - totalSent, 0);
		if (bytesSent <= 0) {

			if  (bytesSent == FATAL)
				std::cerr << "Send failed: (fd " << fd << " ): " << strerror(errno) << std::endl;
			break;
		}
		totalSent += bytesSent;
	}
}

void	Server::sendWelcome(int fd) {

		_clientMap[fd].setAuthenticate(); // Authenticate Client!
		std::string nick = _clientMap[fd].getNickname();
		std::string user = _clientMap[fd].getUsername();
    	sendToClient(fd, ":ircserv " + std::string(RPL_WELCOME) + " " + nick +
			" :Welcome to the IRC Network " + nick + "!" + user + "@" + _clientMap[fd].getHostAdresse() + "\r\n");

}

void	Server::sendChannelWelcome(int fd, Channel& channel) {

	Client& client = _clientMap[fd];

	// Join echo
	sendToClient(fd, ":" + client.getNickname() + "!" + client.getUsername() + "@"
					+ client.getHostAdresse() + " JOIN " + channel.getName() + "\r\n");

	//Topic
	if (channel.getTopic().empty())
		sendToClient(fd, ":ircserv " + std::string(RPL_NOTOPIC) + " " + client.getNickname() + " "
			+ channel.getName() + " :No topic is set\r\n");
	else
	 	sendToClient(fd, ":ircserv " + std::string(RPL_TOPIC) + " " + client.getNickname() + " "
			+ channel.getName() + " :" + channel.getTopic() + "\r\n");

	// Channel members
	std::string userList;
	std::set<int> users = channel.getUsers();
	std::set<int>::iterator it = users.begin();

	for (; it != users.end(); it++) {

		if (channel.isOperator(*it))
			userList += "@";

		userList += _clientMap[*it].getNickname();

		std::set<int>::iterator next = it;
		++next;
		if (next != users.end())
			userList += " ";
	}
	sendToClient(fd, ":ircserv " + std::string(RPL_NAMREPLY) + " " + client.getNickname() + " = "
			+ channel.getName() + " :" + userList + "\r\n");
	sendToClient(fd, ":ircserv " + std::string(RPL_ENDOFNAMES) + " " + client.getNickname() + " "
			+ channel.getName() + " :End of /NAMES list\r\n");

}

void 	Server::broadcastToChannel(int fd, Channel& channel, const std::string& message) {

	std::set<int> users = channel.getUsers();
	std::set<int>::iterator it = users.begin();

	for(; it != users.end(); it++) {

		if (fd == *it)
			continue ;
		sendToClient(*it, message);
	}
}

void 	Server::broadcastQuit(int fd) {

	Client& client = _clientMap[fd];
	const std::set<std::string> clientChannels = client.getJoinedChannels();
	std::set<std::string>::const_iterator it = clientChannels.begin();

	const std::string message = ":" + client.getNickname() + "!"
			+ client.getUsername() + "@" + client.getHostAdresse()
			+ " QUIT :Client Quit\r\n";

	for(; it != clientChannels.end(); it++) {

		Channel& chan = _channels[*it];

		//loescht channel weil letzter user leaved
		if (chan.getUsers().size() == 1)
			_channels.erase(*it);

		//loescht user von channel und broadcastet zu anderen
		else {
			chan.removeUser(fd);
			broadcastToChannel(fd, chan, message);
		}
	}
}


// ───────────────────────────────────────────────
// ──────────────── REGISTRATION ─────────────────
// ───────────────────────────────────────────────

void	Server::handlePass(int fd, const IrcMessage& msg) {

	if (_clientMap[fd].isAuthenticated() || _clientMap[fd].isPassOK()) {
		sendToClient(fd, ":ircserv " + std::string(ERR_ALREADYREGISTERED) +
			" * :You may not reregister\r\n");
		return ;
	}
	else if (msg.params.empty())
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) +
			" * PASS :Not enough parameters\r\n");
	else if (msg.params[0] == _password)
		_clientMap[fd].setPassOK();
	else
		sendToClient(fd, ":ircserv " + std::string(ERR_PASSWDMISMATCH) +
			" * :Password incorrect\r\n");

	if (_clientMap[fd].isPassOK() && _clientMap[fd].isNickOK() && _clientMap[fd].isUserOK() )
		sendWelcome(fd);

}

void	Server::handleNick(int fd, const IrcMessage& msg) {

	Client& client = _clientMap[fd];
	std::string target = client.isNickOK() ? client.getNickname() : "*";

	if (msg.params.empty()) {

		sendToClient(fd, ":ircserv " + std::string(ERR_NONICKNAMEGIVEN) + " "
			+ target  + " :No nickname given\r\n");
		return ;
	}

	else if (client.isNickOK() && msg.params[0] == client.getNickname()) //eigener nick kein fehler
    	return;

	else if (!_parser.isValidNickname(msg.params[0])) {

		sendToClient(fd, ":ircserv " + std::string(ERR_ERRONEUSNICKNAME) + " "
			+ target + " " + msg.params[0] + " :Erroneous nickname\r\n");
		return ;

	}

	else if (_parser.isNicknameInUse(msg.params[0], _clientMap)) {
		//TODO nicknames sind cases insesitive!!
		sendToClient(fd, ":ircserv " + std::string(ERR_NICKNAMEINUSE) + " "
			+ target  + " " + msg.params[0] + " :Nickname is already in use\r\n");
		return ;
	}


	if (client.isAuthenticated()) {

		// 	TODO: inform all other clients in channel about nickchange
		//	broadcastNickChange in all active channel
		// 	format :oldnick!user@host NICK :newnick

		sendToClient(fd, ":" + client.getNickname() + "!" + client.getUsername() + "@host NICK :" + msg.params[0] + "\r\n");
		client.setNickname(msg.params[0]);
		return ;
	}
	client.setNickname(msg.params[0]);
	client.setNickOK();

	if (client.isPassOK() && client.isNickOK() && client.isUserOK() )
		sendWelcome(fd);

}

void	Server::handleUser(int fd, const IrcMessage& msg) {

	if (_clientMap[fd].isAuthenticated() || _clientMap[fd].isUserOK()) {
    	sendToClient(fd, ":ircserv " + std::string(ERR_ALREADYREGISTERED) +
			" * :You may not reregister\r\n");
   		return;
	}
	if (msg.params.size() < 4) {

		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) +
			" * USER :Not enough parameters\r\n");
		return ;
	}

	_clientMap[fd].setUsername(msg.params[0]);
	_clientMap[fd].setUserOK();

	if (_clientMap[fd].isPassOK() && _clientMap[fd].isNickOK() && _clientMap[fd].isUserOK() )
		sendWelcome(fd);
}

// ───────────────────────────────────────────────
// ──────────────── CHANNEL COMMANDS ─────────────
// ───────────────────────────────────────────────
// handleJoin,  handlePrivmsg

void Server::handleJoin(int fd, const IrcMessage& msg) {

    if (msg.params.empty()) {
        sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " "
			+ _clientMap[fd].getNickname() + " JOIN :Not enough parameters\r\n");
        return;
    }

	std::vector<std::string> keys;
    std::vector<std::string> channels = _parser.splitByComma(msg.params[0]);
	if (msg.params.size() > 1)
		keys = _parser.splitByComma(msg.params[1]);


    for (size_t i = 0; i < channels.size(); i++) {

        if (!_parser.isValidChannelName(channels[i])) {
        	sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHCHANNEL) + " "
				+ channels[i] + " :No such channel\r\n");
			continue ;

        }

        std::map<std::string, Channel>::iterator it = _channels.find(channels[i]);
		Channel& chan = _channels[channels[i]];
		std::string key = (i < keys.size()) ? keys[i] : "";

		// Channel wird neu erstellt
		if (it == _channels.end()){

			chan = Channel(channels[i]);
			chan.setKey(key);						//setzt pw
			chan.addUser(fd);						//add user to userlist in channel
			_clientMap[fd].addChannel(channels[i]); //wichtig fuer NICK aenderungen zum broadcoasten an andere clients
			chan.addOperator(fd);
			sendChannelWelcome(fd, chan);
		}

		// Channel existiert bereits
		else {

			if(chan.isMember(fd))
				continue ;

			//prueft inivtie only per operator
			if(chan.getInviteOnly() && !chan.isInvited(fd)) {
				sendToClient(fd, ":ircserv " + std::string(ERR_INVITEONLYCHAN) + " "
					+ channels[i] + " :Cannot join channel (+i)\r\n");
				continue ;
			}

			//prueft ob noch platz im channel frei ist!
			else if(chan.getUserLimit() != 0 && chan.getUsers().size() >= chan.getUserLimit()) {

				sendToClient(fd, ":ircserv " + std::string(ERR_CHANNELISFULL) + " "
					+ channels[i] + " :Cannot join channel (+l)\r\n");
				continue ;
			}

			// passwort protected channels
			else if (!chan.getKey().empty() && key != chan.getKey()) {

				sendToClient(fd, ":ircserv " + std::string(ERR_BADCHANNELKEY) + " "
				+ channels[i] + " :Cannot join channel (+k)\r\n");
				continue ;
			}

			chan.addUser(fd);
			_clientMap[fd].addChannel(channels[i]);
			sendChannelWelcome(fd, chan);

			Client& client = _clientMap[fd];
			const std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@"
				+ client.getHostAdresse() + " JOIN " + chan.getName() + "\r\n";
			broadcastToChannel(fd, chan, message);
		}

    }
}

void	Server::handlePrivmsg(int fd, const IrcMessage& msg) {

	if (msg.params.size() < 2) {
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " "
			+ _clientMap[fd].getNickname() + " PRIVMSG :Not enough parameters\r\n");
        return;
	}


	// TODO multi channel msg ???
	// KICK #chan1,#chan2,#chan3 user1,user2,user3
	std::map<std::string, Channel>::iterator it = _channels.find(msg.params[0]);
	Client& client = _clientMap[fd];

	// Is parmas[0] a channel ??
	if (it != _channels.end()) {

		Channel& chan = it->second;
		// is client member of the channel ??
		if (chan.isMember(fd)) {

			const std::string message = (":" + client.getNickname() + "!"
					+ client.getUsername() + "@" + client.getHostAdresse()
					+ " PRIVMSG " + chan.getName() + " :" + msg.params[1] + "\r\n");

			broadcastToChannel(fd, chan, message);
		}
		// member not found
		else {
			sendToClient(fd, ":ircserv " + std::string(ERR_CANNOTSENDTOCHAN)
			+ " " + chan.getName() + " :Cannot send to channel\r\n");
		}
		return;
	}

	// an client privat senden
	ClientMap::iterator iter = _clientMap.begin();
	for(; iter != _clientMap.end(); iter++) {

		if (iter->second.getNickname() == msg.params[0] && iter->second.isAuthenticated()) {

			const std::string message = (":" + client.getNickname() + "!"
					+ client.getUsername() + "@" + client.getHostAdresse()
					+ " PRIVMSG " + client.getNickname() + " :" + msg.params[1] + "\r\n");

			sendToClient(iter->first, message);
			return ;
		}
	}

	//channelname oder username not found
	sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHNICK)
			+ " " + msg.params[0] + " :No such nick/channel\r\n");

}

// TODO muss channel auch & handlen zb &channel statt #channel??
// ───────────────────────────────────────────────
// ──────────────── OPERATOR COMMANDS ────────────
// ───────────────────────────────────────────────
// handleKick, handleInvite, handleTopic, handleMode

void	Server::handleKick(int fd, const IrcMessage& msg) {

	Client& client = _clientMap[fd];

	if (msg.params.size() < 2) {
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " "
			+ client.getNickname() + " KICK :Not enough parameters\r\n");
        return;
	}

	Channels::iterator it;
	if (!getValidatedChannel(fd, msg.params[0], it))
		return;

	Channel& channel = it->second;
	std::vector<std::string> users = _parser.splitByComma(msg.params[1]);

	for (size_t i = 0; i < users.size(); i++) {

		int userFd;
		if (!getValidatedTargetUser(fd, channel, users[i], userFd, true))
			continue ;

		std::string comment = (msg.params.size() > 2) ? msg.params[2] : users[i];
		const std::string message = (":" + client.getNickname() + "!"
				+ client.getUsername() + "@" + client.getHostAdresse()
				+ " KICK " + channel.getName() + " " + users[i] + " :" + comment + "\r\n");

		broadcastToChannel(fd, channel, message);
		channel.removeUser(userFd);
		_clientMap[userFd].removeChannel(msg.params[0]);
	}
}


void	Server::handleInvite(int fd, const IrcMessage& msg) {


	Client& client = _clientMap[fd];

	if (msg.params.size() < 2) {
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " "
			+ client.getNickname() + " INVITE :Not enough parameters\r\n");
        return;
	}

	Channels::iterator it;
	if (!getValidatedChannel(fd, msg.params[1], it))
		return;

	Channel& channel = it->second;
	std::vector<std::string> users = _parser.splitByComma(msg.params[0]);

	for (size_t i = 0; i < users.size(); i++) {

		int userFd;
		if (!getValidatedTargetUser(fd, channel, users[i], userFd, false))
			continue ;

		if (channel.getInviteOnly())
			channel.addToInviteList(userFd);

		sendToClient(fd, ":ircserv " + std::string(RPL_INVITING) + " "
			+ client.getNickname() + " " + _clientMap[userFd].getNickname() + " "
			+ channel.getName() + "\r\n");

		sendToClient(userFd, ":" + client.getNickname() + "!"
			+ client.getUsername() + "@" + client.getHostAdresse()
			+ " INVITE " + _clientMap[userFd].getNickname() +
			" :" + channel.getName() + "\r\n");

	}
}

void	Server::handleTopic(int fd, const IrcMessage& msg) {

	Client& client = _clientMap[fd];

	if (msg.params.size() < 1) {
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " "
			+ client.getNickname() + " TOPIC :Not enough parameters\r\n");
        return;
	}

	Channels::iterator it;
	if (!getValidatedChannel(fd, msg.params[0], it))
		return;

	Channel& channel = it->second;

	if (msg.params.size() == 1) {

		if (channel.getTopic().empty())
			sendToClient(fd, ":ircserv " + std::string(RPL_NOTOPIC) + " " + client.getNickname() + " "
				+ channel.getName() + " :No topic is set\r\n");

		else
			sendToClient(fd, ":ircserv " + std::string(RPL_TOPIC) + " "
				+ client.getNickname() + " " + channel.getName()
				+ " :" + channel.getTopic() + "\r\n");
        return;
	}

	if (!channel.getTopicProtection()) {

		std::string message = ":" + client.getNickname() + "!"
			+ client.getUsername() + "@" + client.getHostAdresse()
			+ " TOPIC " + channel.getName() + " :" + msg.params[1] + "\r\n";


		sendToClient(fd, message);
		broadcastToChannel(fd, channel, message);
		channel.setTopic(msg.params[1]);

		return;

	}
	else
		sendToClient(fd, ":ircserv " + std::string(ERR_CHANOPRIVSNEEDED) + " "
			+ client.getNickname() + " " + channel.getName() +
			+ " :You're not channel operator\r\n");

}



////
////		OPERATOR helpers
////


bool	Server::getValidatedChannel(int fd, const std::string& channelName, Channels::iterator& outIt) {

	Client& client = _clientMap[fd];

	outIt = _channels.find(channelName);
	if (outIt == _channels.end()) {

		sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHCHANNEL) + " "
			+ client.getNickname() + " " + channelName
			+ " :No such channel\r\n");
		return false;
	}

	if (!outIt->second.isMember(fd)) {

		sendToClient(fd, ":ircserv " + std::string(ERR_NOTONCHANNEL) + " "
			+ client.getNickname() + " " + channelName
			+ " :You're not on that channel\r\n");
		return false;
	}

	if (!outIt->second.isOperator(fd)) {

		sendToClient(fd, ":ircserv " + std::string(ERR_CHANOPRIVSNEEDED) + " "
			+ client.getNickname() + " " + channelName
			+ " :You're not channel operator\r\n");
		return false;
	}

	return true;
}

bool	Server::getValidatedTargetUser(int fd, Channel& channel, const std::string& nickname,
										int& outUserFd, bool requireMember) {

	Client& client = _clientMap[fd];

	outUserFd = getFdByNickname(nickname);
	if (outUserFd == FATAL) {

		sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHNICK) + " "
			+ client.getNickname() + " " + nickname
			+ " :No such nick/channel\r\n");
		return false;
	}

	// KICK Ziel muss Mitglied sein.
	if (requireMember && !channel.isMember(outUserFd)) {

		sendToClient(fd, ":ircserv " + std::string(ERR_USERNOTINCHANNEL) + " "
			+ client.getNickname() + " " + nickname + " " + channel.getName()
			+ " :They aren't on that channel\r\n");
		return false;
	}

	//INVITE Ziel darf noch nicht Mitglied sein.
	if (!requireMember && channel.isMember(outUserFd)) {

		sendToClient(fd, ":ircserv " + std::string(ERR_USERONCHANNEL) + " "
			+ client.getNickname() + " " + nickname + " " + channel.getName()
			+ " :is already on channel\r\n");
		return false;
	}

	return true;
}

int	Server::getFdByNickname(std::string name) const {

	ClientMap::const_iterator it = _clientMap.begin();

	for (; it != _clientMap.end(); it++) {

		if (it->second.getNickname() == name)
			return (it->first);
	}
	return (-1);
}


