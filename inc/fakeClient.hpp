#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <map>

class Client {
	private:
		int					_fd;
		std::string 		_username;
		std::string			_nickname;

	public:
		Client();
		Client(int fd);
		Client(const Client &other);
		Client &operator=(const Client &other);
		~Client();

		int getFd() const;
		std::string getUsername() const;
		std::string getNickname() const;

		void setUsername(const std::string &username);
		void setNickname(const std::string &nickname);

};

typedef std::map<int, Client> ClientMap;

#endif
