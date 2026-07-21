#ifndef IRCMESSAGE_HPP
#define IRCMESSAGE_HPP

#include <string>
#include <vector>

// ───────────────────────────────────────────────
// ──────────────── WELCOME REPLIES ──────────────
// ───────────────────────────────────────────────

#define RPL_WELCOME             "001" // :Welcome to the IRC Network <nick>
#define RPL_YOURHOST            "002" // :Your host is <server>
#define RPL_CREATED             "003" // :This server was created <date>
#define RPL_MYINFO              "004" // <server> <version> <usermodes> <chanmodes>

// ───────────────────────────────────────────────
// ─────────────── CHANNEL REPLIES ───────────────
// ───────────────────────────────────────────────

#define RPL_CHANNELMODEIS       "324" // <channel> <mode> <mode params>
#define RPL_NOTOPIC             "331" // <channel> :No topic is set
#define RPL_TOPIC               "332" // <channel> :<topic>
#define RPL_INVITING            "341" // <channel> <nick>
#define RPL_NAMREPLY            "353" // = <channel> :<nicks>
#define RPL_ENDOFNAMES          "366" // <channel> :End of /NAMES list

// ───────────────────────────────────────────────
// ──────────────── ERROR REPLIES ────────────────
// ───────────────────────────────────────────────

#define ERR_NOSUCHNICK          "401" // <nick> :No such nick/channel
#define ERR_NOSUCHCHANNEL       "403" // <channel> :No such channel
#define ERR_CANNOTSENDTOCHAN    "404" // <channel> :Cannot send to channel
#define ERR_NORECIPIENT         "411" // :No recipient given
#define ERR_NOTEXTTOSEND        "412" // :No text to send
#define ERR_UNKNOWNCOMMAND      "421" // <command> :Unknown command
#define ERR_NONICKNAMEGIVEN     "431" // :No nickname given
#define ERR_ERRONEUSNICKNAME    "432" // <nick> :Erroneous nickname
#define ERR_NICKNAMEINUSE       "433" // <nick> :Nickname is already in use
#define ERR_USERNOTINCHANNEL    "441" // <nick> <channel> :They aren't on that channel
#define ERR_NOTONCHANNEL        "442" // <channel> :You're not on that channel
#define ERR_USERONCHANNEL       "443" // <nick> <channel> :is already on channel
#define ERR_NOTREGISTERED       "451" // :You have not registered
#define ERR_NEEDMOREPARAMS      "461" // <command> :Not enough parameters
#define ERR_ALREADYREGISTERED   "462" // :You may not reregister
#define ERR_PASSWDMISMATCH      "464" // :Password incorrect
#define ERR_CHANNELISFULL       "471" // <channel> :Cannot join channel (+l)
#define ERR_INVITEONLYCHAN      "473" // <channel> :Cannot join channel (+i)
#define ERR_UNKNOWNMODE      	"472" // <nick> <char> :is unknown mode char to me
#define ERR_BADCHANNELKEY       "475" // <channel> :Cannot join channel (+k)
#define ERR_CHANOPRIVSNEEDED    "482" // <channel> :You're not channel operator

struct IrcMessage {
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
};

#endif
