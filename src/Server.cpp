
#include "../inc/Server.hpp"
#include <arpa/inet.h> // htons(), inet_ntop()
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>  // fcntl(), O_NONBLOCK
#include <iostream> // close()
#include <poll.h>   // poll(), struct pollfd
#include <signal.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h> // socket(), bind(), listen(), accept()
#include <unistd.h>     // close()
#include <vector>

void print(std::string str);

// ───────────────────────────────────────────────
// ────────────────── CANONICAL ──────────────────
// ───────────────────────────────────────────────

Server::Server(std::string port, std::string password) {

	_serverSocket = -1;
	char *endptr;
	long tmpPort = strtol(port.c_str(), &endptr, 10);
	if (*endptr != '\0' || tmpPort <= 1024 || tmpPort > 65535)
	    throw std::runtime_error("Port: invalid");
	_port = static_cast<uint16_t>(tmpPort);
  	_password = password;
  	_running = false;

}

Server::Server(const Server &other)
    : _serverSocket(other._serverSocket), _port(other._port),
      _password(other._password), _running(other._running) {}

Server &Server::operator=(const Server &other) {

  if (this != &other) {

    _serverSocket = other._serverSocket;
    _port = other._port;
    _password = other._password;
    _running = other._running;
  }
  return *this;
}

Server::~Server() { cleanSockets(); }

// ───────────────────────────────────────────────
// ─────────────────── SIGNALS ───────────────────
// ───────────────────────────────────────────────

static void signalHandler(int sig) {

	(void)sig;
	if (g_server)
		g_server->stop();
}

void Server::init_signals() {
  struct sigaction sa;

  sa.sa_handler = signalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGINT, &sa, NULL);
  signal(SIGPIPE, SIG_IGN);
}

void printServerStop() {

	struct timespec ts = {0, 300000000L}; // 300ms
	for (int i = 0; i < 10; i++) {
	  const char *dots[] = {"   ", ".  ", ".. ", "..."};
	  std::cout << "\rServer shutting down" << dots[i % 4] << std::flush;
	  nanosleep(&ts, NULL);
	}
	std::cout << "\r                        \r" << std::flush;
}

void Server::stop() {

  if (!_running)
    return;
  _running = false;
}

inline void Server::cleanSockets() {

	if(!_clientMap.empty()) {

		ClientMap::iterator it = _clientMap.begin();

		for(; it != _clientMap.end(); it++) {

			if (it->first >= 0) {

				sendToClient(it->first, "Connection lost to " + it->second.getHostAdresse() + "\r\n");
				close(it->first);
			}
		}

	}
	if (_serverSocket != -1) {
	  close(_serverSocket);
	  _serverSocket = -1;
	}
	printServerStop();
}

// ───────────────────────────────────────────────
// ──────────────────── SETUP ────────────────────
// ───────────────────────────────────────────────

void Server::setup() {

	init_signals();

	std::time_t now = std::time(NULL);
	_createdAt = std::ctime(&now);

	//delete newline at end of time string because of \r\n from server
	if (!_createdAt.empty() && _createdAt[_createdAt.size() - 1] == '\n')
		_createdAt.erase(_createdAt.size() - 1);

	// Socket erstellen
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == FATAL)
	  throw std::runtime_error(std::string("Error socket(): ") +
	                           strerror(errno));

	// Socket konfigurieren (SO_REUSEADDR)
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error(std::string("Error setsockopt(): ") +
	                           strerror(errno));

	// Non-blocking setzen
	if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) == FATAL)
	  throw std::runtime_error(std::string("Error fcntl(): ") + strerror(errno));

	// Port/IP in Netzwerk-Byteorder umwandeln
	memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//	An Port binden
	if (bind(_serverSocket, (struct sockaddr *)&_addr, sizeof(_addr)) == FATAL)
	  throw std::runtime_error(std::string("Error bind(): ") + strerror(errno));

	  // Auf Verbindungen warten
	if (listen(_serverSocket, SOMAXCONN) == FATAL)
	  throw std::runtime_error(std::string("Error listen(): ") + strerror(errno));

	_running = true;
}

// ───────────────────────────────────────────────
// ─────────────────── POLL LOOP ─────────────────
// ───────────────────────────────────────────────

void Server::run() {

	std::vector<pollfd> fds;

	struct pollfd serverfd;         // ein pollfd-Eintrag für den Server-Socket
	serverfd.fd = _serverSocket;    // welcher fd überwacht werden soll
	serverfd.events = POLLIN;       // worauf gewartet wird: "lesbar" = neue Verbindung wartet

	fds.push_back(serverfd);        // in die Liste aller überwachten fds aufnehmen

	while(_running) {

		int polls = poll(fds.data(), fds.size(), -1);
		if (polls == 0)
			throw std::runtime_error("Error poll: system call timed out"); //redundant -> poll(timeout = -1)
		if (polls < 0) {
			if (!_running)
        		break;  // Signal interrupt with control + c
			throw std::runtime_error(std::string("Error poll: ") + strerror(errno));
		}

		std::vector<pollfd> addClients;
		for (iterator socket = fds.begin(); socket != fds.end(); socket++) {

			if (!(socket->revents & POLLIN) && !(socket->revents & POLLHUP))
    			continue;  // skip rest wenn KEINE Events

			// Neuer client wird geadded
			if (socket->fd == _serverSocket){

  				struct sockaddr_in clientAddr;
  				socklen_t clientSize = sizeof(clientAddr);
  				memset(&clientAddr, 0, clientSize);

  				int clientSocket = accept(_serverSocket, (sockaddr *)&clientAddr, &clientSize);
  				if (clientSocket == FATAL)
  				  throw std::runtime_error(std::string("Error accept(): ") +
  				                           strerror(errno));

				// Client Non-blocking setzen
  				if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) == FATAL)
  				  throw std::runtime_error(std::string("Error fcntl(): ") + strerror(errno));

  				_clientMap[clientSocket] =	Client(clientSocket);

				//poll struct
				struct pollfd clientfd;
				clientfd.fd = clientSocket;
				clientfd.events = POLLIN;

				addClients.push_back(clientfd); //adde clients nach dem for loop um foor loop size nicht zu veraendern

				char ip[INET_ADDRSTRLEN];
				if (inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip)) == NULL)
				    std::cout << "New connection (unknown ip)" << std::endl;
				else
				    std::cout << "New connection from " << ip << ":" << ntohs(clientAddr.sin_port) << std::endl;
				_clientMap[clientSocket].setHostAdresse(ip);
			}

			// Ereignis auf CLIENT
			else {

				char buffer[4096];
  				memset(buffer, 0, sizeof(buffer));

				int bytesReceived = recv(socket->fd, buffer, sizeof(buffer) - 1, 0);
				if (bytesReceived <= 0) {

					if (bytesReceived == 0)
						std::cout << "Client disconnected" << std::endl;

					else
						std::cerr << "recv failed: " << strerror(errno) << std::endl;

					broadcastQuit(socket->fd);			// sends quit message to all joined channels
					_clientMap.erase(socket->fd);  		// Client destructor wird gecalled und closed _clientfd
					_parser.clearClient(socket->fd);	// loescht den eintrag in der map fuer IRCMessage
					close(socket->fd);           		// Server schließt FD
					socket = fds.erase(socket);

					socket--;              				// Schleife macht danach socket++, das gleicht das aus
					continue;
				}

				// parse Line
				std::vector<IrcMessage> msgs = _parser.processBuffer(socket->fd, std::string(buffer, bytesReceived));
				for (size_t i = 0; i < msgs.size(); i++)
				    handleCommand(socket->fd, msgs[i]);

			}
		}

		for (size_t i = 0; i < addClients.size(); i++)
    		fds.push_back(addClients[i]);
	}
}
