/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjelinek <pjelinek@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/29 16:33:26 by pjelinek          #+#    #+#             */
/*   Updated: 2026/06/29 19:21:29 by pjelinek         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//#include "Server.hpp"

#include <exception>
#include <iostream>
#include <string>

void print(std::string str) {

	std::cout << str << std::endl;
}

int	main(int ac, char* argv[]) {

	if (ac != 3) {
		print("./ircserv <port> <password>");
		return 1;
	}
	try {
		Server server(av[1], av[2]);
	}
	catch (std::exception &e) {
		print(e.what());
	}
}
