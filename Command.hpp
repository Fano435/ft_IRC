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
    void ignore(Client* client, const std::vector<std::string>& args);
    void handleNick(Client* client, const std::vector<std::string>& args);
    void handleUser(Client* client, const std::vector<std::string>& args);
    void handlePass(Client* client, const std::vector<std::string>& args);
    void quit(Client* client, const std::vector<std::string>& args);
    void ping(Client* client, const std::vector<std::string>& args);
    void handleMessage(Client* client, const std::vector<std::string>& args);
    void handleNotice(Client* client, const std::vector<std::string>& args);
    void join(Client* client, const std::vector<std::string>& args);
    void part(Client* client, const std::vector<std::string>& args);
    void handleKick(Client* client, const std::vector<std::string>& args);
    void handleMode(Client* client, const std::vector<std::string>& args);
    void disconnect(Client *client, const std::string& reason);
    void topic(Client* client, const std::vector<std::string>& args);
    void handleInvite(Client* client, const std::vector<std::string>& args);

public:
    Command(Server &server);
    bool execute(Client *client, const std::string &line);
    ~Command();
};

#endif