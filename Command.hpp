#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <map>
#include <vector>
#include <iostream>

class Server;
class Client;

class Command
{
    typedef void (Command::*CommandHandler)(Client*, const std::vector<std::string>&);
private:
    Server &_server;
    Command();
    std::map<std::string, CommandHandler> _commands;
    void cap(Client* client, const std::vector<std::string>& args);
    void handleNick(Client* client, const std::vector<std::string>& args);
    void handleUser(Client* client, const std::vector<std::string>& args);
    void handlePass(Client* client, const std::vector<std::string>& args);
    void quit(Client* client, const std::vector<std::string>& args);
    void ping(Client* client, const std::vector<std::string>& args);
    void message(Client* client, const std::vector<std::string>& args);
    void join(Client* client, const std::vector<std::string>& args);
    void leave(Client* client, const std::vector<std::string>& args);
    void mode(Client* client, const std::vector<std::string>& args);
    void disconnect(Client *client, const std::string& reason);

public:
    Command(Server &server);
    void execute(Client *client, const std::string &line);
    ~Command();
};

#endif