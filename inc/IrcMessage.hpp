#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <vector>

struct IrcMessage {
	std::string prefix;
	std::string command;
	std::vector<std::string> params;	
};

#endif
