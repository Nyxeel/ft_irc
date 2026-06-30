/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/01 00:14:15 by pjelinek         ###   ########.fr       */
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



Server::Server(std::string port, std::string password) {

	// if(Parse::function() == SUCCESS)
		_password = password;
		_port = atoi(port.c_str());

		memset(&_addr, 0, sizeof(_addr));
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(_port);


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


void	Server::setup() {

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (_sockfd > 0)
			throw std::runtime_error("Error: socket() failed");

	/*

	bind()
	listen()

	while(poll())
	{
		acceppt(), connect(),


		recv(), send(), recvfrom(), sendto()
	}
	closesocket()
	*/


}

