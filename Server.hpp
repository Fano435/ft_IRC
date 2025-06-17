#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include "Client.hpp"
#include "error.h"

// class Parser;

#define MAX_EVENTS 10
#define MAX_PORT 65535
#define BUFFER_SIZE 1024


class Server
{
    typedef void (Server::*CommandHandler)(Client*, const std::vector<std::string>&);

private:
    Server();
    int _port;
    int _socket;
    const std::string _password;

    std::map<int, Client *> _clients;
    std::map<int, std::string> _errors;
    std::map<std::string, CommandHandler> _commands;

    void parseLine(Client *client, const std::string &line);
    void handleNick(Client* client, const std::vector<std::string>& args);
    void handleUser(Client* client, const std::vector<std::string>& args);
    void handlePass(Client* client, const std::vector<std::string>& args);
    int createSocket() const;
    void initializeErrorMessages();

public:
    Server(const char *s_port, const std::string password);
    ~Server();
    void run();
};


#endif