
#include "../inc/Bot.hpp"

#include "../inc/Parser.hpp"
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iostream>

//(de)constructor

Bot::Bot(const std::string &name) : _name(name) {
	std::srand(std::time(0));
	_loadDatabase();
}

Bot::~Bot() {

}

//private functions

void Bot::_loadDatabase() {
	std::ifstream file("badwords.txt");
	if (!file.is_open()) {
		std::cerr << "[Bot warning] can't open badwords.txt. Censorship inactive!" << std::endl;
		return;
	}

	std::string word;
	while (std::getline(file, word))
		if (!word.empty())
			_badWordsDb.push_back(word);
	file.close();
}

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

const std::string Bot::getName() const {
	return _name;
}

bool Bot::censorMessage(const std::string &target, std::string &text) {
	if (target.empty() || (target[0] != '#' && target[0] != '&'))
		return false;

	bool censoredAny = false;

	for (size_t i = 0; i < _badWordsDb.size(); ++i) {
		size_t pos;
		while ((pos = text.find(_badWordsDb[i])) != std::string::npos) {
			std::string stars(_badWordsDb[i].length(), '*');
			text.replace(pos, _badWordsDb[i].length(), stars);
			censoredAny = true;
		}
	}
	return censoredAny;
}


bool Bot::processBotMessage(const IrcMessage &msg, std::string &responseText) {
	if (msg.command != "PRIVMSG")
		return false;

	if (responseText[0] != '!')
		return false;

	if (responseText.find("!help") != std::string::npos) {
		responseText = _getHelp();
		return true;
	}
	if (responseText.find("!time") != std::string::npos) {
		responseText = _getTime();
		return true;
	}
	if (responseText.find("!joke") != std::string::npos) {
		responseText = _getJoke();
		return true;
	}
	if (responseText.find("!roll") != std::string::npos) {
		responseText = _rollDice();
		return true;
	}
	if (responseText.find("!flip") != std::string::npos) {
		responseText = _flipCoin();
		return true;
	}

	return false;
}