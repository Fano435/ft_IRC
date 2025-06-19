#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include <iostream>
#include "Client.hpp"
#include "error.h"

// class Parser;

#define MAX_EVENTS 10
#define MAX_PORT 65535
#define BUFFER_SIZE 512


class Server
{
    typedef void (Server::*CommandHandler)(Client*, const std::vector<std::string>&);

private:
    Server();
    int epoll_fd;
    int _port;
    int _socket;
    const std::string _password;
    const std::string _server;

    std::map<int, Client *> _clients;
    std::map<int, std::string> _errors;
    std::map<std::string, CommandHandler> _commands;

    void parseMsg(int sender_sock);
    void parseLine(Client *client, const std::string &line);
    void handleCap(Client* client, const std::vector<std::string>& args);
    void handleNick(Client* client, const std::vector<std::string>& args);
    void handleUser(Client* client, const std::vector<std::string>& args);
    void handlePass(Client* client, const std::vector<std::string>& args);
    int createSocket() const;
    void initReplies();
    void disconnect(Client* client, const std::string& reason);
    void sendRWelcome(Client* client);
    void sendError(Client* client, int code, const std::string& param);

public:
    Server(const char *s_port, const std::string password);
    ~Server();
    void run();
};


#endif