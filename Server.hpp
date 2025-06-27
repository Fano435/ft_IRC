#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include <iostream>
#include "Client.hpp"
#include "errors.hpp"

class Command;

#define MAX_EVENTS 10
#define MAX_PORT 65535

#define BUFFER_SIZE 512
#define SERVER "localhost"

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

    Command *_command;
    std::map<int, Client *> _clients;

    void parseMsg(int sender_sock);
    void parseLine(Client *client, const std::string &line);
    int  createSocket() const;
    
public:
    void disconnect(Client* client, const std::string& reason);
    Server(const char *s_port, const std::string password);
    ~Server();
    void run();
    std::map<int, Client *> &getClients();
    const std::string getPassword() const;
};


#endif