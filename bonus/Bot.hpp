#ifndef BOT_HPP
#define BOT_HPP

#include "IrcMessage.hpp"
#include <string>
#include <vector>

class Bot {
	private:
		std::string					_name;
		std::vector <std::string>	_badWordsDb;

		Bot(const Bot &other);
		Bot &operator=(const Bot &other);

		void		_loadDatabase();
		std::string _getHelp() const;
		std::string _getTime() const;
		std::string _getJoke() const;
		std::string _rollDice() const;
		std::string _flipCoin() const;

	public:
		Bot(const std::string &name = "Bot");
		~Bot();

		std::string getName() const;

		bool censorMessage(const std::string &target, std::string &text);
		bool processBotMessage(const IrcMessage &msg, std::string &responseText);
};

#endif