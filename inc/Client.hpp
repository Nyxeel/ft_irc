/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:21:46 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/18 14:21:36 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <map>
#include <string>

#define SUCCESS 0
#define ERROR 	1
#define FATAL	-1

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
	bool					_passOK;
	bool					_userOK;
	bool					_nickOK;


	public:
		Client();
		Client(int clientSocket);
		Client(const Client &other);
		Client& operator=(const Client &other);
		~Client();

		int					getClientSocket() const;
		std::string			getNickname() const;
		std::string			getUsername() const;

		void				setNickname(const std::string& nickname);
		void				setUsername(const std::string& username);
		void				setAuthenticate();

		void				setPassOK();
		void				setNickOK();
		void				setUserOK();

		bool				isPassOK() const;
		bool				isNickOK() const;
		bool				isUserOK() const;
		bool				isAuthenticated() const;


};

typedef std::map<int, Client> ClientMap;

#endif
