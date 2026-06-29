/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:43:53 by pjelinek          #+#    #+#             */
/*   Updated: 2026/06/29 19:52:09 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

Server::Server(std::string port, std::string password) :
		port(port), password(password) {

	//Parse::function();


}

Server::Server(const Server &other) :
		port(other.port), password(other.password) {

}

Server& Server::operator=(const Server &other) {

	if (this != &other) {

		port = other.port;
		password = other.password;

	}
	return *this;
}

Server::~Server() {


}


