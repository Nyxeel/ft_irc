/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 16:33:26 by pjelinek          #+#    #+#             */
/*   Updated: 2026/07/01 00:30:48 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

#include <exception>
#include <iostream>
#include <string>

void print(std::string str) {

	std::cout << str << std::endl;
}

int	main(int ac, char* av[]) {

	if (ac != 3) {
		print("Expected -> ./ircserv <port> <password>");
		return 1;
	}
	try {
		Server server(av[1], av[2]);
		server.setup();
	}
	catch (std::exception &e) {
		print(e.what());
	}
}
