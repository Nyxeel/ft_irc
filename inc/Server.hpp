/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 19:21:46 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/20 12:32:23 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <string>
#include <netinet/in.h>   	// struct sockaddr_in, INADDR_ANY
#include <vector>
#include <poll.h>   		// poll(), struct pollfd

#include "Client.hpp"
#include "Channel.hpp"
#include "Parser.hpp"
#include "IrcMessage.hpp"

#define SUCCESS 0
#define ERROR 	1
#define FATAL	-1
#define MAX_CLIENTS 1024

#define CMD_PASS	"PASS"
#define CMD_NICK	"NICK"
#define CMD_USER	"USER"
#define CMD_JOIN	"JOIN"
#define CMD_PART	"PART"
#define CMD_PRIVMSG	"PRIVMSG"
#define CMD_QUIT	"QUIT"
#define CMD_KICK	"KICK"
#define CMD_INVITE	"INVITE"
#define CMD_TOPIC	"TOPIC"
#define CMD_MODE	"MODE"

class Server {

	private :

		typedef std::vector<pollfd>::iterator iterator;

		int						_serverSocket;
		uint16_t 				_port;
		std::string 			_password;

		struct sockaddr_in		_addr;

		void					init_signals();
		void					cleanSockets();
		bool					_running;

		ClientMap				_clientMap;
		Parser					_parser;
		Channels				_channels;

		void					handleCommand(int fd, const IrcMessage& msg);
		void    				handlePass(int fd, const IrcMessage& msg);
		void    				handleNick(int fd, const IrcMessage& msg);
		void    				handleUser(int fd, const IrcMessage& msg);
		void    				handleJoin(int fd, const IrcMessage& msg);
		void    				handlePrivmsg(int fd, const IrcMessage& msg);
		void    				handlePart(int fd, const IrcMessage& msg);
		void    				handleQuit(int fd, const IrcMessage& msg);
		void    				handleKick(int fd, const IrcMessage& msg);
		void    				handleInvite(int fd, const IrcMessage& msg);
		void    				handleTopic(int fd, const IrcMessage& msg);
		void    				handleMode(int fd, const IrcMessage& msg);
		void					sendToClient(int fd, const std::string& msg);
		void					sendWelcome(int fd);
		void					sendChannelWelcome(int fd, Channel& chan);
		void 					broadcastToChannel(int fd, Channel& chan, const std::string& message);

		bool					getChannel();

		//isActiveClient()


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

