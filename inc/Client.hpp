/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:21:46 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/17 09:42:45 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLINET_HPP
# define CLINET_HPP

#include <map>
#include <string>
#include "Channel.hpp"

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


class Client {

	int						_clientSocket;
	std::string				_nickname;
	std::string				_username;
	bool					_authenticate;
	std::map<int, Channel> 	_channel;
	std::string				_buffer;

	public:
		Client();
		Client(int clientSocket);
		Client(const Client &other);
		Client& operator=(const Client &other);
		~Client();

	void        appendBuffer(const std::string& data) { _buffer += data; }
    std::string& getBuffer() { return _buffer; }


};


#endif
