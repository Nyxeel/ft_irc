/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:21:46 by pjelinek          #+#    #+#             */
/*   Updated: 2026/06/29 21:12:11 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <string>
#include <netinet/in.h>   	// struct sockaddr_in, INADDR_ANY


class Server {

	int					_fd;
	uint16_t 			_port;
	std::string 		_password;
	struct sockaddr_in	_addr;

	public:
		Server(std::string port, std::string password);
		Server(const Server &other);
		Server& operator=(const Server &other);
		~Server();



};





#endif
