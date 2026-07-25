
#include "../inc/Server.hpp"

#include <exception>
#include <iostream>
#include <string>

Server* 		g_server = NULL;

inline void print(std::string str) {

	std::cerr << str << std::endl;
}

int	main(int ac, char* av[]) {

	if (ac != 3) {
		print("Expected -> ./ircserv <port> <password>");
		return 0;
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
