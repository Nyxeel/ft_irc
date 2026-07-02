/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/02 12:46:44 by pjelinek         ###   ########.fr       */
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
#include <netdb.h>  // NI_MAXHOST, NI_MAXSERV, getnameinfo()
#include <poll.h>   // poll(), struct pollfd
#include <signal.h>
#include <stdexcept>
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <unistd.h>     // close()

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
  sa.sa_flags = SA_RESTART; // TODO: SA_RESTART in conflict with poll?

  sigaction(SIGINT, &sa, NULL);
  signal(SIGPIPE, SIG_IGN);
}

// ───────────────────────────────────────────────
// ───────────────────── STOP ────────────────────
// ───────────────────────────────────────────────

void printServerStop() {

  struct timespec ts = {0, 300000000L}; // 300ms

  for (int i = 0; i < 8; i++) {
    const char *dots[] = {"   ", ".  ", ".. ", "..."};
    std::cout << "\rServer shutting down" << dots[i % 4] << std::flush;
    nanosleep(&ts, NULL);
  }
  std::cout << "\r                        \r" << std::flush;
}

void Server::stop() {

  if (!_running)
    return;
  // TODO: close all client sockets
  _running = false;
}

inline void Server::cleanSockets() {

  if (_serverSocket != -1) {
    close(_serverSocket);
    _serverSocket = -1;
  }

  printServerStop();
  // TODO: close all client sockets
}

// ───────────────────────────────────────────────
// ──────────────────── SETUP ────────────────────
// ───────────────────────────────────────────────

void Server::setup() {

  // Socket erstellen
  _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (_serverSocket == FATAL)
    throw std::runtime_error(std::string("Error serverSocket: ") +
                             strerror(errno));

  // Socket konfigurieren (SO_REUSEADDR)
  int opt = 1;
  if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ==
      -1)
    throw std::runtime_error(std::string("Error setsockopt: ") +
                             strerror(errno));

  // Non-blocking setzen
  if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) == FATAL)
    throw std::runtime_error(std::string("Error fcntl: ") + strerror(errno));

  // Port/IP in Netzwerk-Byteorder umwandeln
  memset(&_addr, 0, sizeof(_addr));
  _addr.sin_family = AF_INET;
  _addr.sin_port = htons(_port);
  _addr.sin_addr.s_addr = htonl(INADDR_ANY);

  //	An Port binden
  if (bind(_serverSocket, (struct sockaddr *)&_addr, sizeof(_addr)) == FATAL)
    throw std::runtime_error(std::string("Error bind: ") + strerror(errno));

  // Auf Verbindungen warten
  if (listen(_serverSocket, SOMAXCONN) == FATAL)
    throw std::runtime_error(std::string("Error listen: ") + strerror(errno));

  _running = true;
}

void Server::run() {

  struct sockaddr_in clientAddr;
  socklen_t clientSize = sizeof(clientAddr);

  int clientSocket =
      accept(_serverSocket, (sockaddr *)&clientAddr, &clientSize);
  if (clientSocket == FATAL)
    throw std::runtime_error(std::string("Error clientSocket: ") +
                             strerror(errno));

  Client client(clientSocket);
  _clientMap[clientSocket] = client;
  close(_serverSocket);

  char host[NI_MAXHOST];
  char service[NI_MAXSERV];

  memset(host, 0, NI_MAXHOST);
  memset(service, 0, NI_MAXSERV);

  int result = getnameinfo((sockaddr *)&clientAddr, sizeof(clientAddr), host,
                           NI_MAXHOST, service, NI_MAXSERV, 0);

  if (result != 0) {
    std::cerr << "getnameinfo failed: " << strerror(result) << std::endl;
  } else {
    std::cout << "New connection from " << host << ":" << service << std::endl;
  }

  char buffer[4096];
  while (_running) {

    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == 0) {

		std::cout << "Client disconnected" << std::endl;
      	break;

    } else if (bytesReceived < 0) {

		std::cerr << "recv failed: " << strerror(errno) << std::endl;
    	break;
    }

	std::cout << "Received: " << buffer << std::endl;


	// parsen muss auf \r\n enden, sonst ist es kein kompletter command
	// Parse::function(buffer, bytesReceived);


	// zurücksenden
	




  }

  // Wenn ich hier den code erreiche, hab ich dann noch zugriff auf den client
  // in _clientMap ??

  /* 	poll()


    recv(), send(), recvfrom(), sendto() */
}
