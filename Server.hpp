#ifndef SERVER_HPP
# define SERVER_HPP

# include "Client.hpp"
# include "errors.hpp"
# include <iostream>
# include <map>
# include <vector>

class	Command;
class	Channel;

# define MAX_EVENTS 10
# define MAX_PORT 65535

# define BUFFER_SIZE 512
# define SERVER "127.0.0.1"

class Server
{
	typedef void (Server::*CommandHandler)(Client *,
		const std::vector<std::string> &);
	typedef std::map<std::string, Channel *>::iterator chan_iterator;
	typedef std::map<int, Client *>::iterator client_iterator;

  private:
	Server();
	int epoll_fd;
	int _port;
	int _socket;
	const std::string _password;
	const std::string _server;

	Command *_command;
	std::map<int, Client *> _clients;
	std::map<std::string, Channel *> _channels;

	void parseMsg(int sender_sock);
	int createSocket() const;

  public:
	void disconnect(Client *client, const std::string &reason);
	void addToChannel(Client *client, std::string name);
	void messageChannel(Client *client, std::string name,
		const std::string &msg);
	void removeFromChannel(Client *client, std::string name,
		const std::string &msg);
	void removeFromAll(Client *client);
	void changeMode(Client *client, const std::vector<std::string> &args);
	void reply(Client *receiver, const std::string &reply);
	Server(const char *s_port, const std::string password);
	~Server();
	void run();
	std::map<int, Client *> &getClients();
	const std::string getPassword() const;
};

#endif