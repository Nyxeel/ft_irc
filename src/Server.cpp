/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/06/29 21:15:12 by pjelinek         ###   ########.fr       */
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

Server::Server(std::string port, std::string password) {

	// if(Parse::function())
		_password = password;
		_port = atoi(port.c_str());

		memset(&_addr, 0, sizeof(_addr));
		uint16_t socket_addr = htons(_port);

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


