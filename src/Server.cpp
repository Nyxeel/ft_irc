/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/01 07:51:59 by pjelinek         ###   ########.fr       */
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



// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────

Server::Server(std::string port, std::string password) {

	// if(Parse::function() == SUCCESS)
		_password = password;
		_port = atoi(port.c_str());
		init_signals();

}

Server::Server(const Server &other) :
		_port(other._port), _password(other._password) {

}

Server& Server::operator=(const Server &other) {

	if (this != &other) {

		_port = other._port;
		_password = other._password;

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


	//[2] socket()                    ← Socket erstellen
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (_sockfd > 0)
			throw std::runtime_error("Error: socket() failed");


//[3] setsockopt()                ← Socket konfigurieren (SO_REUSEADDR)
//[4] fcntl()                     ← Non-blocking setzen

	//[5] htons / htonl               ← Port/IP in Netzwerk-Byteorder umwandeln
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
//[6] bind()                      ← An Port binden
//[7] listen()                    ← Auf Verbindungen warten


	_running = true;
/*
	while(_running)
	{
		poll()
		acceppt(), connect(),


		recv(), send(), recvfrom(), sendto()
	}

	*/


}

