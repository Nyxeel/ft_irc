
#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <map>
#include <set>
#include <string>

#define SUCCESS 0
#define ERROR 	1
#define FATAL	-1
#define INET_ADDRSTRLEN 16



class Client {

	int						_clientSocket;
	std::string				_nickname;
	std::string				_username;

	bool					_authenticate;
	bool					_passOK;
	bool					_userOK;
	bool					_nickOK;
	std::string				_ip;
	std::set<std::string>	_joinedChannels;



	public:
		Client();
		Client(int clientSocket);
		~Client();

		int						getClientSocket() const;
		std::string				getNickname() const;
		std::string				getUsername() const;
		std::string				getHostAdresse() const;
		std::set<std::string> 	getJoinedChannels() const;

		void 					addChannel(const std::string& name);
		void 					removeChannel(const std::string& name);

		void					setNickname(const std::string& nickname);
		void					setUsername(const std::string& username);
		void					setAuthenticate();
		void					setHostAdresse(char ip[INET_ADDRSTRLEN]);

		void					setPassOK();
		void					setNickOK();
		void					setUserOK();

		bool					isPassOK() const;
		bool					isNickOK() const;
		bool					isUserOK() const;
		bool					isAuthenticated() const;


};

typedef std::map<int, Client> ClientMap;


#endif

// 	irssi commands
//
//

// 	### Server-Commands – gehen in handleCommand() an echte Handler ###
// 	(Reihenfolge wie im if/else in Server.commands.cpp)
//
// 	PASS <password>                        /connect <host> <port> <password>  Server-Passwort bei der Registrierung senden
// 	NICK <nickname>                        /nick <nickname>                    Eigenen Nicknamen setzen/ändern
// 	USER <user> <mode> <unused> :<real>    (automatisch bei Verbindungsaufbau) Username/Realname bei der Registrierung senden
// 	JOIN <channel>[,<channel2>...]         /join <channel>                     Channel betreten (legt ihn an, falls er noch nicht existiert)
// 	PRIVMSG <target> :<text>               /msg <nick/channel> <text>          Private oder Channel-Nachricht senden
// 	KICK <channel> <nick> [:reason]        /kick <nick> [reason]               User aus einem Channel werfen (nur Operator)
// 	INVITE <nick> <channel>                /invite <nick> [channel]            User zu einem (invite-only) Channel einladen
// 	TOPIC <channel> [:topic]               /topic [neues topic]                Channel-Topic anzeigen oder setzen
// 	MODE <target> <modes> [params]         /mode <channel/nick> <modes>        Channel- oder User-Modes setzen/abfragen (z.B. +i, +o, +k)

/home/netrunner/projects/ft_irc/README.md
// 	### DCC Filetransfer – relevante Befehle ###
//
// 	/dcc send <nick> <file>          Datei an <nick> senden
// 	/dcc get <nick> <file>           Angebotene Datei annehmen/downloaden
// 	/dcc close send <nick> <file>    Sende-Transfer abbrechen
// 	/dcc close get <nick> <file>     Empfangs-Transfer abbrechen
// 	/dcc list                        Alle offenen DCC-Verbindungen anzeigen
//
// 	### Relevante Einstellungen ###
//
// 	/set dcc_download_path           Zeigt/setzt Download-Ordner
// 	/set dcc_upload_path             Standard-Ordner beim Senden (relativer Pfad)
// 	/set dcc_port_range <von>-<bis>  Portbereich für DCC-Verbindungen
// 	/set dcc_autorename ON/OFF       Bei Namenskonflikt neue Datei mit Suffix
// 	/set dcc_autoresume ON/OFF       Automatisch fortsetzen statt neu anlegen
// 	/set dcc_autoget ON/OFF          Dateien automatisch ohne Nachfrage annehmen
