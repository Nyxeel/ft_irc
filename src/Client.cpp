/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/17 06:11:34 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Client.hpp"
#include <arpa/inet.h> // htons()...
#include <arpa/inet.h> // htons(), inet_ntop()
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h> // fcntl(), O_NONBLOCK
#include <poll.h>  // poll(), struct pollfd
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <unistd.h>     // close()
#include <iostream>     // close()
#include <string>


void print(std::string str);

// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────

Client::Client() {

	_clientSocket = -1;
	_nickname = "";
	_username = "";
	_authenticate = false;
	_buffer = NULL;

}

Client::Client(int clientSocket) {

	_clientSocket 	= clientSocket;
	_nickname 		= "";
	_username 		= "";
	_authenticate 	= false;
	_buffer 		= NULL;

}

Client::Client(const Client &other) :
	_clientSocket(other._clientSocket), _nickname(other._nickname),
	_username(other._username), _authenticate(other._authenticate),
	_channel(other._channel), _buffer(other._buffer)
{

}

Client &Client::operator=(const Client &other) {

  if (this != &other) {

	_clientSocket 	=	other._clientSocket;
	_nickname 		=	other._nickname;
	_username 		=	other._username;
	_authenticate 	=	other._authenticate;
	_channel 		=	other._channel;
	_buffer 		=	other._buffer;
  }
  return *this;
}

Client::~Client() {

}
