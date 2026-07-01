/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:21:46 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/01 14:57:04 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <string>
#include <netinet/in.h>   	// struct sockaddr_in, INADDR_ANY

#define SUCCESS 0
#define ERROR 	1

/*
		struct sockaddr_in {
		    sa_family_t    sin_family;   // Adressfamilie
		    in_port_t      sin_port;     // Port (Network Byte Order)
		    struct in_addr sin_addr;     // IPv4-Adresse
		    char           sin_zero[8];  // Padding (ungenutzt)
		};

		struct in_addr {
		    uint32_t s_addr;             // IPv4 als 32-Bit Zahl
		};
*/


class Server {

	int					_sockfd;
	uint16_t 			_port;
	std::string 		_password;
	struct sockaddr_in	_addr;

	void				init_signals();
	bool				_running;


	public:
		Server(std::string port, std::string password);
		Server(const Server &other);
		Server& operator=(const Server &other);
		~Server();

		void	setup();
		void	stop();
		void	run();

};

extern Server* g_server;



#endif
