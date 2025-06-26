#ifndef COMMAND_HPP
#define COMMAND_HPP

#pragma once

#include <map>
#include <vector>
#include <iostream>
class Server;
class Client;

class Command
{
    typedef void (Command::*CommandHandler)(Client*, const std::vector<std::string>&);
private:
    Command();
    Command &operator=(const Command &rhs);
    Server &_server;
    std::map<std::string, CommandHandler> _commands;
    void handleCap(Client* client, const std::vector<std::string>& args);
    void handleNick(Client* client, const std::vector<std::string>& args);
    void handleUser(Client* client, const std::vector<std::string>& args);
    void handlePass(Client* client, const std::vector<std::string>& args);
    
public:
    Command(Server &server);
    void execute(Client *client, const std::string &line);
    ~Command();
};

#endif