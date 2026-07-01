/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/01 08:38:20 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"
#include <arpa/inet.h>		// htons()...
#include <sys/socket.h>   	// socket(), bind(), listen(), accept()
#include <arpa/inet.h>    	// htons(), inet_ntop()
#include <poll.h>         	// poll(), struct pollfd
#include <unistd.h>       	// close()
#include <fcntl.h>        	// fcntl(), O_NONBLOCK
#include <cstring>
#include <stdexcept>
#include <cstdlib>
#include <signal.h>
#include <cerrno>



// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────

Server::Server(std::string port, std::string password) {

	// if(Parse::function() == SUCCESS)

		_sockfd = -1;
		_port = atoi(port.c_str());
		_password = password;
		_running = false;
		init_signals();

}

Server::Server(const Server &other) :
		_sockfd(other._sockfd), _port(other._port),
		_password(other._password), _running(other._running) {

}

Server& Server::operator=(const Server &other) {

	if (this != &other) {

		_sockfd = other._sockfd;
		_port = other._port;
		_password = other._password;
		_running = other._running;
	}
	return *this;
}

Server::~Server() {

}


// ───────────────────────────────────────────────
// ─────────────────── SIGNALS ───────────────────
// ───────────────────────────────────────────────

static void	signalHandler(int sig) {

	(void) sig;
	if(g_server)
		g_server->stop();
}

void Server::init_signals() {
    struct sigaction sa;

    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT,  &sa, NULL);
	signal(SIGPIPE, SIG_IGN);
}



// ───────────────────────────────────────────────
// ───────────────────── STOP ────────────────────
// ───────────────────────────────────────────────

void	Server::stop() {

	if (_running) {

	//close(socketFD)
	//close(clientFDs) von accept

		_running = false;
	}

}



// ───────────────────────────────────────────────
// ──────────────────── SETUP ────────────────────
// ───────────────────────────────────────────────

void	Server::setup() {


	//Socket erstellen
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd == -1)
		throw std::runtime_error(std::string ("Error socket: ") + strerror(errno));


	//Socket konfigurieren (SO_REUSEADDR)
	int opt = 1;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error(std::string ("Error setsockopt: ") + strerror(errno));

	//Non-blocking setzen
	if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error(std::string ("Error fcntl: ") + strerror(errno));

	//Port/IP in Netzwerk-Byteorder umwandeln
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);


	//	An Port binden
	if (bind(_sockfd, (struct sockaddr*)&_addr, sizeof(_addr)) == -1)
		throw std::runtime_error(std::string ("Error bind: ") + strerror(errno));

	// Auf Verbindungen warten
	if (listen(_sockfd, SOMAXCONN) == -1)
		throw std::runtime_error(std::string ("Error listen: ") + strerror(errno));

	_running = true;
}


void	Server::run() {

	/*
	while(_running)
	{
		poll()
		acceppt(), connect(),


		recv(), send(), recvfrom(), sendto()
	}

	*/

}
