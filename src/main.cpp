/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 16:33:26 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/01 15:37:57 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

#include <exception>
#include <iostream>
#include <string>

Server* g_server = NULL;

inline void print(std::string str) {

	std::cout << str << std::endl;
}

int	main(int ac, char* av[]) {

	if (ac != 3) {
		print("Expected -> ./ircserv <port> <password>");
		return 1;
	}
	try {
		Server server(av[1], av[2]);
		g_server = &server;
		server.setup();
		server.run();
	}
	catch (std::exception &e) {
		print(e.what());
		return 1;
	}
	return 0;
}
