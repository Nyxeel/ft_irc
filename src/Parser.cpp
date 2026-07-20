#include "../inc/Parser.hpp"
#include "../inc/IrcMessage.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <cctype>

//(de)constructor

Parser::Parser() {

}

Parser::~Parser() {

}

//private functions

void Parser::_parseSingleLine(const std::string &line, IrcMessage &msg) {
	std::string s = line;
	_trimString(s);

	if (s.empty()) return;

	if (!s.empty() && s[0] == ':') {
		size_t spacePos = s.find(' ');
		if (spacePos != std::string::npos) {
			msg.prefix = s.substr(1, spacePos - 1);
			s.erase(0, spacePos + 1);
			_trimString(s);
		}
	}

	size_t trailingPos = s.find(" :");
	std::string trailingPart = "";
	if (trailingPos != std::string::npos) {
		trailingPart = s.substr(trailingPos + 2);
		s = s.substr(0, trailingPos);
	} else if (!s.empty() && s[0] == ':') {
		trailingPart = s.substr(1);
		s = "";
	}

	std::istringstream iss(s);
	std::string token;

	if (iss >> token) {
		msg.command = token;

		while (iss >> token)
			msg.params.push_back(token);
	}

	if (trailingPos != std::string::npos || (!trailingPart.empty() && s.empty()))
		msg.params.push_back(trailingPart);
}

void Parser::_trimString(std::string &str) {
	size_t start = str.find_first_not_of(" \t\r\n");

	if (start == std::string::npos) {
		str = "";
		return;
	}

	size_t end = str.find_last_not_of(" \t\r\n");
	str = str.substr(start, end - start + 1);
}

//public functions

std::vector<IrcMessage> Parser::processBuffer(int clientFd, const std::string &rawData) {
	std::vector<IrcMessage> parsedMessages;

	_clientBuffers[clientFd] += rawData;
	std::string &buffer = _clientBuffers[clientFd];

	size_t pos;
	while ((pos = buffer.find('\n')) != std::string::npos) {
		std::string line = buffer.substr(0, pos);
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (!line.empty()) {
			IrcMessage msg;
			_parseSingleLine(line, msg);
			parsedMessages.push_back(msg);
		}
		buffer.erase(0, pos + 1);
	}
	return parsedMessages;
}


void Parser::clearClient(int clientFd) {
	_clientBuffers.erase(clientFd);
}

std::vector<std::string> Parser::splitByComma(const std::string &str) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, ',')) {
		tokens.push_back(token); // TODO isempty() entfernt wegen #channel - key in JOIN. sonst noch woanders verwendet ??
	}
	return tokens;
}

bool Parser::isValidNickname(const std::string &nick) {
	if (nick.empty()) return false;

	if (std::isdigit(nick[0]) || nick[0] == '-') return false;

	std::string allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-[]\\|`_^{}";
	for (size_t i = 0; i < nick.length(); ++i)
		if (allowed.find(nick[i]) == std::string::npos) return false;

	return true;
}

bool Parser::isValidUsername(const std::string &user) {
	if (user.empty() || user.length() > 9) return false; // TODO lenght anpassen falls notwendig

	if (std::isdigit(user[0]) || user[0] == '-') return false;

	std::string allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-[]\\|`_^{}";
	for (size_t i = 0; i < user.length(); ++i)
		if (allowed.find(user[i]) == std::string::npos) return false;

	return true;
}

bool Parser::isValidChannelName(const std::string &channel) {
	if (channel.empty() || channel.length() < 2 || channel.length() > 50) return false;

	if (channel[0] != '#' && channel[0] != '&') return false;

	for (size_t i = 0; i < channel.length(); ++i)
		if (channel[i] == ' ' || channel[i] == ',' || channel[i] == '\a')
			return false;

	return true;
}

static char toIrcLower(char c) {
	if (c >= 'A' && c <= 'Z') return c + 32;
	if (c == '[') return '{';
	if (c == ']') return '}';
	if (c == '\\') return '|';
	if (c == '~') return '^';
	return c;
}

bool Parser::ircStringCompare(const std::string &s1, const std::string &s2) {
	if (s1.length() != s2.length()) return false;

	for (size_t i = 0; i < s1.length(); ++i)
		if (toIrcLower(s1[i]) != toIrcLower(s2[i]))
			return false;

	return true;
}

bool Parser::isNicknameInUse(const std::string &nickname, const ClientMap &clients) {

	ClientMap::const_iterator it;

	for (it = clients.begin(); it != clients.end(); ++it)
		if (ircStringCompare(it->second.getNickname(), nickname))
			return true;

	return false;
}
