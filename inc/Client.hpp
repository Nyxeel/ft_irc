/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:21:46 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/20 16:28:14 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <map>
#include <set>
#include <string>

#define SUCCESS 0
#define ERROR 	1
#define FATAL	-1
#define INET_ADDRSTRLEN 16



class Client {

	int						_clientSocket;
	std::string				_nickname;
	std::string				_username;

	bool					_authenticate;
	bool					_passOK;
	bool					_userOK;
	bool					_nickOK;
	std::string				_ip;
	std::set<std::string>	_joinedChannels;



	public:
		Client();
		Client(int clientSocket);
		~Client();


		int						getClientSocket() const;
		std::string				getNickname() const;
		std::string				getUsername() const;
		std::string				getHostAdresse() const;
		std::set<std::string> 	getJoinedChannels() const;

		void 					addChannel(const std::string& name);
		void 					removeChannel(const std::string& name);

		void					setNickname(const std::string& nickname);
		void					setUsername(const std::string& username);
		void					setAuthenticate();
		void					setHostAdresse(char ip[INET_ADDRSTRLEN]);

		void					setPassOK();
		void					setNickOK();
		void					setUserOK();

		bool					isPassOK() const;
		bool					isNickOK() const;
		bool					isUserOK() const;
		bool					isAuthenticated() const;


};

typedef std::map<int, Client> ClientMap;


#endif
