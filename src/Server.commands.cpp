
#include "../inc/Server.hpp"
#include "../inc/Bot.hpp"
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

void print(std::string str);

void Server::handleCommand(int fd, const IrcMessage &msg)
{

	if (msg.command == CMD_PASS)
		handlePass(fd, msg);
	else if (msg.command == CMD_NICK)
		handleNick(fd, msg);
	else if (msg.command == CMD_USER)
		handleUser(fd, msg);
	else if (msg.command == CMD_CAP)
		return;
	else if (!_clientMap[fd].isAuthenticated())
	{
		// JOIN, PRIVMSG etc. vor Login = Fehler
		sendToClient(fd, ":ircserv " + std::string(ERR_NOTREGISTERED) +
							 " * :You have not registered\r\n");
		return;
	}
	else if (msg.command == CMD_JOIN)
		handleJoin(fd, msg);
	else if (msg.command == CMD_PRIVMSG)
		handlePrivmsg(fd, msg);
	else if (msg.command == CMD_KICK)
		handleKick(fd, msg);
	else if (msg.command == CMD_INVITE)
		handleInvite(fd, msg);
	else if (msg.command == CMD_TOPIC)
		handleTopic(fd, msg);
	else if (msg.command == CMD_MODE)
		handleMode(fd, msg);
	else
		sendToClient(fd, ":ircserv " + std::string(ERR_UNKNOWNCOMMAND) + " " + _clientMap[fd].getNickname() + " " + msg.command + " :Unknown command\r\n");
}

// ───────────────────────────────────────────────
// ──────────────── SEND / BROADCAST ─────────────
// ───────────────────────────────────────────────
// sendToClient, sendWelcome, sendChannelWelcome, broadcastToChannel

void Server::sendToClient(int fd, const std::string &message)
{

	size_t totalSent = 0;
	int bytesSent;

	std::string msg = message;

	if (BONUS)
	{

		Bot bot;
		bot.censorMessage(msg);
	}

	while (totalSent < msg.size())
	{

		bytesSent = send(fd, msg.c_str() + totalSent, msg.size() - totalSent, 0);
		if (bytesSent <= 0)
		{

			if (bytesSent == FATAL)
				std::cerr << "Send failed: (fd " << fd << " ): " << strerror(errno) << std::endl;
			break;
		}
		totalSent += bytesSent;
	}
}

void Server::sendWelcome(int fd)
{

	_clientMap[fd].setAuthenticate(); // Authenticate Client!
	std::string nick = _clientMap[fd].getNickname();
	std::string user = _clientMap[fd].getUsername();
	sendToClient(fd, ":ircserv " + std::string(RPL_WELCOME) + " " + nick +
						 " :Welcome to the IRC Network " + nick + "!" + user + "@" + _clientMap[fd].getHostAdresse() + "\r\n");

	sendToClient(fd, ":ircserv " + std::string(RPL_YOURHOST) + " " + nick +
						 " :Your host is ircserv, running version 1.0\r\n");

	sendToClient(fd, ":ircserv " + std::string(RPL_CREATED) + " " + nick +
						 " :This server was created " + _createdAt + "\r\n");

	sendToClient(fd, ":ircserv " + std::string(RPL_MYINFO) + " " + nick +
						 " ircserv 1.0 - itkol\r\n");
}

void Server::sendChannelWelcome(int fd, Channel &channel)
{

	Client &client = _clientMap[fd];

	// Join echo
	sendToClient(fd, ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " JOIN " + channel.getName() + "\r\n");

	// Topic
	if (channel.getTopic().empty())
		sendToClient(fd, ":ircserv " + std::string(RPL_NOTOPIC) + " " + client.getNickname() + " " + channel.getName() + " :No topic is set\r\n");
	else
		sendToClient(fd, ":ircserv " + std::string(RPL_TOPIC) + " " + client.getNickname() + " " + channel.getName() + " :" + channel.getTopic() + "\r\n");

	// Channel members
	std::string userList;
	std::set<int> users = channel.getUsers();
	std::set<int>::iterator it = users.begin();

	for (; it != users.end(); it++)
	{

		if (channel.isOperator(*it))
			userList += "@";

		userList += _clientMap[*it].getNickname();

		std::set<int>::iterator next = it;
		++next;
		if (next != users.end())
			userList += " ";
	}
	sendToClient(fd, ":ircserv " + std::string(RPL_NAMREPLY) + " " + client.getNickname() + " = " + channel.getName() + " :" + userList + "\r\n");
	sendToClient(fd, ":ircserv " + std::string(RPL_ENDOFNAMES) + " " + client.getNickname() + " " + channel.getName() + " :End of /NAMES list\r\n");
}

void Server::broadcastToChannel(int fd, Channel &channel, const std::string &message)
{

	std::set<int> users = channel.getUsers();
	std::set<int>::iterator it = users.begin();

	for (; it != users.end(); it++)
	{

		if (fd == *it)
			continue;
		std::string output = message;
		if (BONUS)
		{
			Bot bot;
			bot.censorMessage(output);
		}
		sendToClient(*it, output);
	}
}

void Server::broadcastQuit(int fd)
{

	Client &client = _clientMap[fd];
	const std::set<std::string> clientChannels = client.getJoinedChannels();
	std::set<std::string>::const_iterator it = clientChannels.begin();

	const std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " QUIT :Client Quit\r\n";

	for (; it != clientChannels.end(); it++)
	{

		Channel &chan = _channels[*it];

		// loescht channel weil letzter user leaved
		if (chan.getUsers().size() == 1)
			_channels.erase(*it);

		// loescht user von channel und broadcastet zu anderen
		else
		{
			chan.removeUser(fd);
			broadcastToChannel(fd, chan, message);
		}
	}
}

void Server::broadcastNickChange(int fd, const std::string &oldNick)
{

	Client &client = _clientMap[fd];

	const std::string message = ":" + oldNick + "!" + client.getUsername() + "@" + client.getHostAdresse() + " NICK :" + client.getNickname() + "\r\n";

	const std::set<std::string> clientChannels = client.getJoinedChannels();
	std::set<std::string>::const_iterator it = clientChannels.begin();
	std::set<int> notified;

	for (; it != clientChannels.end(); it++)
	{

		Channels::iterator cit = _channels.find(*it);
		if (cit == _channels.end())
			continue;

		const std::set<int> &users = cit->second.getUsers();
		std::set<int>::const_iterator uit = users.begin();

		for (; uit != users.end(); uit++)
		{

			if (*uit == fd || notified.find(*uit) != notified.end())
				continue;
			sendToClient(*uit, message);
			notified.insert(*uit);
		}
	}

	sendToClient(fd, message);
}

// ───────────────────────────────────────────────
// ──────────────── REGISTRATION ─────────────────
// ───────────────────────────────────────────────

void Server::handlePass(int fd, const IrcMessage &msg)
{

	if (_clientMap[fd].isAuthenticated() || _clientMap[fd].isPassOK())
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_ALREADYREGISTERED) +
							 " * :You may not reregister\r\n");
		return;
	}
	else if (msg.params.empty())
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) +
							 " * PASS :Not enough parameters\r\n");
	else if (msg.params[0] == _password)
		_clientMap[fd].setPassOK();
	else
		sendToClient(fd, ":ircserv " + std::string(ERR_PASSWDMISMATCH) +
							 " * :Password incorrect\r\n");

	if (_clientMap[fd].isPassOK() && _clientMap[fd].isNickOK() && _clientMap[fd].isUserOK())
		sendWelcome(fd);
}

void Server::handleNick(int fd, const IrcMessage &msg)
{

	Client &client = _clientMap[fd];

	if (!client.isPassOK())
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NOTREGISTERED) +
							 " * :You have not registered\r\n");
		return;
	}

	std::string target = client.isNickOK() ? client.getNickname() : "*";

	if (msg.params.empty())
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NONICKNAMEGIVEN) + " " + target + " :No nickname given\r\n");
		return;
	}

	else if (client.isNickOK() && msg.params[0] == client.getNickname()) // eigener nick kein fehler
		return;

	else if (!_parser.isValidNickname(msg.params[0]))
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_ERRONEUSNICKNAME) + " " + target + " " + msg.params[0] + " :Erroneous nickname\r\n");
		return;
	}

	else if (_parser.isNicknameInUse(msg.params[0], _clientMap))
	{
		// TODO nicknames sind cases insesitive!!
		sendToClient(fd, ":ircserv " + std::string(ERR_NICKNAMEINUSE) + " " + target + " " + msg.params[0] + " :Nickname is already in use\r\n");
		return;
	}

	if (client.isAuthenticated())
	{

		std::string oldNick = client.getNickname();
		client.setNickname(msg.params[0]);
		broadcastNickChange(fd, oldNick);
		return;
	}
	client.setNickname(msg.params[0]);
	client.setNickOK();

	if (client.isPassOK() && client.isNickOK() && client.isUserOK())
		sendWelcome(fd);
}

void Server::handleUser(int fd, const IrcMessage &msg)
{

	Client &client = _clientMap[fd];

	if (!client.isPassOK())
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NOTREGISTERED) +
							 " * :You have not registered\r\n");
		return;
	}

	if (client.isAuthenticated() || client.isUserOK())
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_ALREADYREGISTERED) +
							 " * :You may not reregister\r\n");
		return;
	}
	if (msg.params.size() < 4)
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) +
							 " * USER :Not enough parameters\r\n");
		return;
	}

	client.setUsername(msg.params[0]);
	client.setUserOK();

	if (client.isPassOK() && client.isNickOK() && client.isUserOK())
		sendWelcome(fd);
}

// ───────────────────────────────────────────────
// ──────────────── CHANNEL COMMANDS ─────────────
// ───────────────────────────────────────────────
// handleJoin,  handlePrivmsg

void Server::handleJoin(int fd, const IrcMessage &msg)
{

	if (msg.params.empty())
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " " + _clientMap[fd].getNickname() + " JOIN :Not enough parameters\r\n");
		return;
	}

	std::vector<std::string> keys;
	std::vector<std::string> channels = _parser.splitByComma(msg.params[0]);
	if (msg.params.size() > 1)
		keys = _parser.splitByComma(msg.params[1]);

	for (size_t i = 0; i < channels.size(); i++)
	{

		if (!_parser.isValidChannelName(channels[i]))
		{
			sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHCHANNEL) + " " + _clientMap[fd].getNickname() + " " + channels[i] + " :No such channel\r\n");
			continue;
		}

		std::map<std::string, Channel>::iterator it = _channels.find(channels[i]);
		Channel &chan = _channels[channels[i]];
		std::string key = (i < keys.size()) ? keys[i] : "";

		// Channel wird neu erstellt
		if (it == _channels.end())
		{

			chan = Channel(channels[i]);
			chan.setKey(key);						// setzt pw
			chan.addUser(fd);						// add user to userlist in channel
			_clientMap[fd].addChannel(channels[i]); // wichtig fuer NICK aenderungen zum broadcoasten an andere clients
			chan.setOperator(fd);
			sendChannelWelcome(fd, chan);
		}

		// Channel existiert bereits
		else
		{

			if (chan.isMember(fd))
				continue;

			// prueft inivtie only per operator
			if (chan.getInviteOnly() && !chan.isInvited(fd))
			{
				sendToClient(fd, ":ircserv " + std::string(ERR_INVITEONLYCHAN) + " " + _clientMap[fd].getNickname() + " " + channels[i] + " :Cannot join channel (+i)\r\n");
				continue;
			}

			// prueft ob noch platz im channel frei ist!
			else if (chan.getUserLimit() != 0 && chan.getUsers().size() >= chan.getUserLimit())
			{

				sendToClient(fd, ":ircserv " + std::string(ERR_CHANNELISFULL) + " " + _clientMap[fd].getNickname() + " " + channels[i] + " :Cannot join channel (+l)\r\n");
				continue;
			}

			// passwort protected channels
			else if (!chan.getKey().empty() && key != chan.getKey())
			{

				sendToClient(fd, ":ircserv " + std::string(ERR_BADCHANNELKEY) + " " + _clientMap[fd].getNickname() + " " + channels[i] + " :Cannot join channel (+k)\r\n");
				continue;
			}

			chan.addUser(fd);
			_clientMap[fd].addChannel(channels[i]);
			sendChannelWelcome(fd, chan);

			Client &client = _clientMap[fd];
			const std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " JOIN " + chan.getName() + "\r\n";
			broadcastToChannel(fd, chan, message);
		}
	}
}

void Server::handlePrivmsg(int fd, const IrcMessage &msg)
{

	if (msg.params.empty())
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NORECIPIENT) + " " + _clientMap[fd].getNickname() + " :No recipient given\r\n");
		return;
	}

	if (msg.params.size() < 2)
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NOTEXTTOSEND) + " " + _clientMap[fd].getNickname() + " :No text to send\r\n");
		return;
	}

	std::vector<std::string> targets = _parser.splitByComma(msg.params[0]);
	Client &client = _clientMap[fd];

	for (size_t i = 0; i < targets.size(); i++)
	{

		std::map<std::string, Channel>::iterator it = _channels.find(targets[i]);

		// Is target[i] a channel ??
		if (it != _channels.end())
		{

			Channel &chan = it->second;
			// is client member of the channel ??
			if (chan.isMember(fd))
			{
				bool flag = false;
				std::string response = msg.params[1];

				if (BONUS) {

					Bot bot;
					flag = bot.processBotMessage(response);
				}

				const std::string message = (":" + client.getNickname() + "!"
					+ client.getUsername() + "@" + client.getHostAdresse()
					+ " PRIVMSG " + chan.getName() + " :"
					+ response + "\r\n");
				if (flag)
					sendToClient(fd, message);

				broadcastToChannel(fd, chan, message);
			}
			// member not found
			else
			{
				sendToClient(fd, ":ircserv " + std::string(ERR_CANNOTSENDTOCHAN) + " " + client.getNickname() + " " + chan.getName() + " :Cannot send to channel\r\n");
			}
			continue;
		}

		// an client privat senden
		ClientMap::iterator iter = _clientMap.begin();

		bool found = false;
		for (; iter != _clientMap.end(); iter++)
		{

			if (iter->second.getNickname() == targets[i] && iter->second.isAuthenticated())
			{

				std::string response = msg.params[1];

				if (BONUS) {

					Bot bot;

					//print("Bot processing message: " + response);
					bot.processBotMessage(response);
				}

				const std::string message = (":" + client.getNickname() + "!"
					+ client.getUsername() + "@" + client.getHostAdresse()
					+ " PRIVMSG " + iter->second.getNickname() + " :"
					+ response + "\r\n");

				sendToClient(iter->first, message);
				found = true;
				break;
			}
		}

		// channelname oder username not found
		if (!found)
			sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHNICK) + " " + client.getNickname() + " " + targets[i] + " :No such nick/channel\r\n");
	}
}

// ───────────────────────────────────────────────
// ──────────────── OPERATOR COMMANDS ────────────
// ───────────────────────────────────────────────
// handleKick, handleInvite, handleTopic, handleMode

void Server::handleKick(int fd, const IrcMessage &msg)
{

	Client &client = _clientMap[fd];

	if (msg.params.size() < 2)
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " " + client.getNickname() + " KICK :Not enough parameters\r\n");
		return;
	}

	Channels::iterator it;
	if (!getValidatedChannel(fd, msg.params[0], it))
		return;

	Channel &channel = it->second;
	std::string channelName = msg.params[0];
	std::vector<std::string> users = _parser.splitByComma(msg.params[1]);

	for (size_t i = 0; i < users.size(); i++)
	{

		int userFd;
		if (!getValidatedTargetUser(fd, channel, users[i], userFd, true))
			continue;

		std::string comment = (msg.params.size() > 2) ? msg.params[2] : users[i];
		const std::string message = (":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " KICK " + channel.getName() + " " + users[i] + " :" + comment + "\r\n");

		sendToClient(fd, message);
		broadcastToChannel(fd, channel, message);
		_clientMap[userFd].removeChannel(channelName);

		// loescht channel weil letzter user gekickt wurde
		if (channel.getUsers().size() == 1)
		{
			_channels.erase(channelName);
			break;
		}
		channel.removeUser(userFd);
	}
}

void Server::handleInvite(int fd, const IrcMessage &msg)
{

	Client &client = _clientMap[fd];

	if (msg.params.size() < 2)
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " " + client.getNickname() + " INVITE :Not enough parameters\r\n");
		return;
	}

	Channels::iterator it;
	if (!getValidatedChannel(fd, msg.params[1], it))
		return;

	Channel &channel = it->second;
	std::vector<std::string> users = _parser.splitByComma(msg.params[0]);

	for (size_t i = 0; i < users.size(); i++)
	{

		int userFd;
		if (!getValidatedTargetUser(fd, channel, users[i], userFd, false))
			continue;

		if (channel.getInviteOnly())
			channel.addToInviteList(userFd);

		sendToClient(fd, ":ircserv " + std::string(RPL_INVITING) + " " + client.getNickname() + " " + _clientMap[userFd].getNickname() + " " + channel.getName() + "\r\n");

		sendToClient(userFd, ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " INVITE " + _clientMap[userFd].getNickname() +
								 " :" + channel.getName() + "\r\n");
	}
}

void Server::handleTopic(int fd, const IrcMessage &msg)
{

	Client &client = _clientMap[fd];

	if (msg.params.size() < 1)
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
		return;
	}

	// validate channel
	Channels::iterator it;
	it = _channels.find(msg.params[0]);
	if (it == _channels.end())
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHCHANNEL) + " " + client.getNickname() + " " + msg.params[0] + " :No such channel\r\n");
		return;
	}

	Channel &channel = it->second;
	// read TOPIC
	if (msg.params.size() == 1 && channel.isMember(fd))
	{

		if (channel.getTopic().empty())
			sendToClient(fd, ":ircserv " + std::string(RPL_NOTOPIC) + " " + client.getNickname() + " " + channel.getName() + " :No topic is set\r\n");

		else
			sendToClient(fd, ":ircserv " + std::string(RPL_TOPIC) + " " + client.getNickname() + " " + channel.getName() + " :" + channel.getTopic() + "\r\n");
		return;
	}

	// SET new topic
	if ((!channel.getTopicProtection() && channel.isMember(fd)) || channel.isOperator(fd))
	{

		std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " TOPIC " + channel.getName() + " :" + msg.params[1] + "\r\n";

		sendToClient(fd, message);
		broadcastToChannel(fd, channel, message);
		channel.setTopic(msg.params[1]);

		return;
	}
	if (!channel.isMember(fd))
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NOTONCHANNEL) + " " + client.getNickname() + " " + msg.params[0] + " :You're not on that channel\r\n");
		return;
	}
	if (!channel.isOperator(fd))
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_CHANOPRIVSNEEDED) + " " + client.getNickname() + " " + msg.params[0] + " :You're not channel operator\r\n");
		return;
	}
}

void Server::handleMode(int fd, const IrcMessage &msg)
{

	Client &client = _clientMap[fd];

	if (msg.params.empty())
	{
		sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " " + client.getNickname() + " MODE :Not enough parameters\r\n");
		return;
	}

	// irssi silencer - irssi after authent if
	if (msg.params[0] == client.getNickname())
		return;

	Channels::iterator it;
	it = _channels.find(msg.params[0]);
	if (it == _channels.end())
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHCHANNEL) + " " + client.getNickname() + " " + msg.params[0] + " :No such channel\r\n");
		return;
	}

	Channel &channel = it->second;
	if (!channel.isMember(fd))
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NOTONCHANNEL) + " " + client.getNickname() + " " + channel.getName() + " :You're not on that channel\r\n");
		return;
	}

	// MODE #channel ohne modestring -> aktuelle Modes anzeigen
	if (msg.params.size() < 2)
	{

		std::string modes = "+";
		std::string params;

		if (channel.getInviteOnly())
			modes += "i";
		if (channel.getTopicProtection())
			modes += "t";
		if (!channel.getKey().empty())
		{
			modes += "k";
			params += " " + channel.getKey();
		}
		if (channel.getUserLimit() != 0)
		{
			modes += "l";
			std::ostringstream oss;
			oss << channel.getUserLimit();
			params += " " + oss.str();
		}

		sendToClient(fd, ":ircserv " + std::string(RPL_CHANNELMODEIS) + " " + client.getNickname() + " " + channel.getName() + " " + modes + params + "\r\n");
		return;
	}

	if (!channel.isOperator(fd))
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_CHANOPRIVSNEEDED) + " " + client.getNickname() + " " + channel.getName() + " :You're not channel operator\r\n");
		return;
	}

	const std::string &modestring = msg.params[1];
	char mode = '+';

	size_t args = msg.params.size() - 2;
	size_t count = 0;

	for (size_t i = 0; i < modestring.size(); i++)
	{

		if (modestring[i] == '+' || modestring[i] == '-')
		{

			mode = modestring[i];
			continue;
		}
		switch (modestring[i])
		{

		case 'i':
		{

			channel.setInviteOnly(mode == '+');

			std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " MODE " + channel.getName() + " " + mode + "i\r\n";

			sendToClient(fd, message);
			broadcastToChannel(fd, channel, message);
			break;
		}
		case 't':
		{

			channel.setTopicProtection(mode == '+');

			std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " MODE " + channel.getName() + " " + mode + "t\r\n";

			sendToClient(fd, message);
			broadcastToChannel(fd, channel, message);
			break;
		}
		case 'k':
		{

			std::string key = "";
			if (mode == '+')
			{

				if (count >= args)
				{
					sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " " + client.getNickname() + " MODE " + mode + "k :Not enough parameters\r\n");
					break;
				}
				key = msg.params[2 + count++];
			}
			channel.setKey(key);

			std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " MODE " + channel.getName() + " " + mode + "k" + (mode == '+' ? " " + key : "") + "\r\n";

			sendToClient(fd, message);
			broadcastToChannel(fd, channel, message);
			break;
		}
		case 'o':
		{

			if (count >= args)
			{
				sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " " + client.getNickname() + " MODE " + mode + "o :Not enough parameters\r\n");
				break;
			}
			std::string user = msg.params[2 + count++];

			int userFd = getFdByNickname(user);
			if (!channel.isMember(userFd))
			{

				sendToClient(fd, ":ircserv " + std::string(ERR_USERNOTINCHANNEL) + " " + client.getNickname() + " " + user + " " + channel.getName() + " :They aren't on that channel\r\n");
				break;
			}

			if ((mode == '+' && channel.isOperator(userFd)) || (mode == '-' && !channel.isOperator(userFd)))
				break; // breaken da keine aenderung stattfindet

			else if (mode == '+' && !channel.isOperator(userFd))
				channel.setOperator(userFd);

			else if (mode == '-' && channel.isOperator(userFd))
				channel.removeOperator(userFd);

			std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " MODE " + channel.getName() + " " + mode + "o " + _clientMap[userFd].getNickname() + "\r\n";

			sendToClient(fd, message);
			broadcastToChannel(fd, channel, message);
			break;
		}

		case 'l':
		{

			if (mode == '+')
			{
				if (count >= args || !checkDigits(msg.params[2 + count]))
				{
					sendToClient(fd, ":ircserv " + std::string(ERR_NEEDMOREPARAMS) + " " + client.getNickname() + " MODE " + mode + "l :Not enough parameters\r\n");
					break;
				}

				char *endptr;
				size_t userLimit = std::strtol(msg.params[2 + count].c_str(), &endptr, 10);
				channel.setUserLimit(userLimit);
			}
			else
				channel.setUserLimit(0);

			std::ostringstream oss;
			oss << channel.getUserLimit();

			std::string message = ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostAdresse() + " MODE " + channel.getName() + " " + mode + "l " + oss.str() + "\r\n";

			sendToClient(fd, message);
			broadcastToChannel(fd, channel, message);
			break;
		}

		default:
			sendToClient(fd, ":ircserv " + std::string(ERR_UNKNOWNMODE) + " " + client.getNickname() + " " + modestring[i] + " :is unknown mode char to me\r\n");
		}
	}
}


bool Server::checkDigits(const std::string &str)
{

	for (size_t i = 0; i < str.size(); i++)
	{

		if (!std::isdigit(str[i]))
			return false;
	}
	return true;
}

bool Server::getValidatedChannel(int fd, const std::string &channelName, Channels::iterator &outIt)
{

	Client &client = _clientMap[fd];

	outIt = _channels.find(channelName);
	if (outIt == _channels.end())
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHCHANNEL) + " " + client.getNickname() + " " + channelName + " :No such channel\r\n");
		return false;
	}

	if (!outIt->second.isMember(fd))
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NOTONCHANNEL) + " " + client.getNickname() + " " + channelName + " :You're not on that channel\r\n");
		return false;
	}

	if (!outIt->second.isOperator(fd))
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_CHANOPRIVSNEEDED) + " " + client.getNickname() + " " + channelName + " :You're not channel operator\r\n");
		return false;
	}

	return true;
}

bool Server::getValidatedTargetUser(int fd, Channel &channel, const std::string &nickname,
									int &outUserFd, bool requireMember)
{

	Client &client = _clientMap[fd];

	outUserFd = getFdByNickname(nickname);
	if (outUserFd == FATAL)
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_NOSUCHNICK) + " " + client.getNickname() + " " + nickname + " :No such nick/channel\r\n");
		return false;
	}

	// KICK Ziel muss Mitglied sein.
	if (requireMember && !channel.isMember(outUserFd))
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_USERNOTINCHANNEL) + " " + client.getNickname() + " " + nickname + " " + channel.getName() + " :They aren't on that channel\r\n");
		return false;
	}

	// INVITE Ziel darf noch nicht Mitglied sein.
	if (!requireMember && channel.isMember(outUserFd))
	{

		sendToClient(fd, ":ircserv " + std::string(ERR_USERONCHANNEL) + " " + client.getNickname() + " " + nickname + " " + channel.getName() + " :is already on channel\r\n");
		return false;
	}

	return true;
}

int Server::getFdByNickname(std::string name) const
{

	ClientMap::const_iterator it = _clientMap.begin();

	for (; it != _clientMap.end(); it++)
	{

		if (it->second.getNickname() == name)
			return (it->first);
	}
	return (-1);
}
