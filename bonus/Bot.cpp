#include "Bot.hpp"
#include "../inc/Parser.hpp"
#include <ctime>
#include <cstdlib>
#include <sstream>

//(de)constructor

Bot::Bot(const std::string &name) : _name(name) {
	std::srand(std::time(0));
}

Bot::~Bot() {

}

//private functions

std::string Bot::_getHelp() const {
	return "Usable commands: !help, !time, !joke, !roll (roll a dice), !flip (flip a coin)";
}

std::string Bot::_getTime() const {
	std::time_t now = std::time(0);
	std::string dt = std::ctime(&now);
	if (!dt.empty() && dt[dt.length() - 1] == '\n')
	dt.erase(dt.length() - 1);
	return "Servertime: " + dt;
}

std::string Bot::_getJoke() const {
	const char *jokes[] = {
		"Why do programmers hate getting up? Because they run 'Sleep()' in bed.",
		"There are 10 types of people: those who understand binary and those who don't.",
		"An SQL query walks into a bar, sees two tables, and asks, 'May I join you?'",
		"What do you call a programmer who doesn't swear? Unemployed."
	};
	int index = std::rand() % 4;
	return jokes[index];
}

std::string Bot::_rollDice() const {
	int result = (std::rand() % 6) + 1;
	std::stringstream ss;
	ss << "🎲 You rolled a " << result << "!";
	return ss.str();
}

std::string Bot::_flipCoin() const {
	int result = std::rand() % 2;
	if (result == 0)
		return "🪙 Coinflip: HEAD!";
	else
		return "🪙 Coinflip: TAIL!";
}

//public functions

std::string Bot::getName() const {
	return _name;
}

bool Bot::processBotMessage(const IrcMessage &msg, std::string &responseText) {
	if (msg.command != "PRIVMSG" || msg.params.size() < 2)
		return false;

	std::string target = msg.params[0];
	std::string text = msg.params[1];

	bool isDirectMessage = Parser::ircStringCompare(target, _name);
	if (!isDirectMessage && (text.empty() || text[0] != '!'))
		return false;

	if (text.find("!help") != std::string::npos || (isDirectMessage && text.find("help") != std::string::npos)) {
		responseText = _getHelp();
		return true;
	}
	if (text.find("!time") != std::string::npos || (isDirectMessage && text.find("time") != std::string::npos)) {
		responseText = _getTime();
		return true;
	}
	if (text.find("!joke") != std::string::npos || (isDirectMessage && text.find("joke") != std::string::npos)) {
		responseText = _getJoke();
		return true;
	}
	if (text.find("!roll") != std::string::npos || (isDirectMessage && text.find("roll") != std::string::npos)) {
		responseText = _rollDice();
		return true;
	}
	if (text.find("!flip") != std::string::npos || (isDirectMessage && text.find("flip") != std::string::npos)) {
		responseText = _flipCoin();
		return true;
	}

	if (isDirectMessage) {
		responseText = "Unknown command. Type 'help' for a list of commands.";
		return true;
	}

	return false;
}