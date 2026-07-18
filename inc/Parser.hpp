#ifndef PARSER_HPP
#define PARSER_HPP

#include "IrcMessage.hpp"
#include "Client.hpp"
#include <string>
#include <vector>

class Parser {

	private:

		std::map<int, std::string> _clientBuffers;

		Parser(const Parser &other);
		Parser &operator=(const Parser &other);

		void	_parseSingleLine(const std::string &line, IrcMessage &msg);
		void	_trimString(std::string &str);

	public:

		Parser();
		~Parser();

		std::vector<IrcMessage> 	processBuffer(int clientFd, const std::string &rawData);
		void 						clearClient(int clientFd);
		static 						std::vector<std::string> splitByComma(const std::string &str);
		static bool 				isValidNickname(const std::string &nick);
		static bool 				isValidUsername(const std::string &user);
		static bool 				isValidChannelName(const std::string &channel);
		static bool 				ircStringCompare(const std::string &s1, const std::string &s2);
		static bool 				isNicknameInUse(const std::string &nickname, const ClientMap &clients);
		static bool 				isUsernameInUse(const std::string &username, const ClientMap &clients);
};

#endif
