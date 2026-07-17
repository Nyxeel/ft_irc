/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/17 11:12:32 by pjelinek         ###   ########.fr       */
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

void print(std::string str);

// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────

Server::Server(std::string port, std::string password) {

  // if(Parse::function() == SUCCESS)
  // All ports below 1024 are RESERVED (unless you’re the superuser)! You can
  // have any port number above that, right up to 65535 (provided they aren’t
  // already being used by another program).

  _serverSocket = -1;
  _port = atoi(port.c_str());
  _password = password;
  _running = false;

  init_signals();
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

// ───────────────────────────────────────────────
// ───────────────────── STOP ────────────────────
// ───────────────────────────────────────────────

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
			if (errno == EINTR)
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
				// Ereignis auf einem CLIENT-Socket → Daten lesen
				//reciv() and send()

				char buffer[4096];
  				memset(buffer, 0, sizeof(buffer));
  				int bytesReceived = recv(socket->fd, buffer, sizeof(buffer) - 1, 0);
				if (bytesReceived <= 0) {

					if (bytesReceived == 0)
						std::cout << "Client disconnected" << std::endl;
					else
						std::cerr << "recv failed: " << strerror(errno) << std::endl;

					close(socket->fd);           	// Server schließt FD
					_clientMap.erase(socket->fd);  	// Client destructor wird gecalled und closed _clientfd
					socket = fds.erase(socket);
					socket--;              			// Schleife macht danach socket++, das gleicht das aus
					continue;
				}

				_clientMap[socket->fd].appendBuffer(std::string(buffer, bytesReceived));

				std::string& buf = _clientMap[socket->fd].getBuffer();
				size_t pos;
				while ((pos = buf.find("\r\n")) != std::string::npos) {
				    std::string cmd = buf.substr(0, pos);
				    buf.erase(0, pos + 2);
				    //handleCommand(socket->fd, cmd);
				}
				std::cout << "Received: " << buffer << std::endl;

			}
		}

		for (size_t i = 0; i < addClients.size(); i++)
    			fds.push_back(addClients[i]);
	}
}
