#include "Command.hpp"
#include "Client.hpp"
#include "server.h"
#include <sstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Command::Command(Server &server) : _server(server)
{
    _commands["CAP"] = &Command::handleCap;
    _commands["PASS"] = &Command::handlePass;
    _commands["NICK"] = &Command::handleNick;
    _commands["USER"] = &Command::handleUser;

}

Command::~Command()
{

}

bool isValidNickname(const std::string& nick) {
    if (nick.empty())
        return false;

    char first = nick[0];

    if (first == ':' || first == '#' || std::isdigit(first))
        return false;

    for (size_t i = 0; i < nick.length(); ++i) {
        char c = nick[i];

        if (std::isalnum(c))
            continue;

        if (c == '[' || c == ']' ||
            c == '{' || c == '}' ||
            c == '\\' || c == '|')
            continue;

        if (std::isspace(c))
            return false;

        return false;
    }

    return true;
}

void Command::handleNick(Client* client, const std::vector<std::string>& args)
{
    if (!client->isAuthenticated())
        return ;
    std::string nick = args[0];
    std::map<int, Client *> clients = _server.getClients();
    for (std::map<int, Client *>::iterator it = clients.begin(); it != clients.end(); it++)
    {
        if ((*it).second->getNickname() == nick)
        {
            sendError(client, ERR_NICKNAMEINUSE);
            return ;
        }
    }
    if(!isValidNickname(nick))
    {
        sendError(client, ERR_ERRONEUSNICKNAME);
        return ;
    }
    client->setNickname(nick);
}

void Command::handleCap(Client* client, const std::vector<std::string>& args)
{
    (void)client;
    (void)args;
}

void Command::handleUser(Client* client, const std::vector<std::string>& args)
{
    if (!client->isAuthenticated())
        return ;
    if (client->isRegistered())
    {
        sendError(client, ERR_ALREADYREGISTRED);
        return ;
    }
    std::string user = args[0];
    if (user.empty())
    {
        sendError(client, ERR_NEEDMOREPARAMS);
        return ;
    }
    client->setUsername(args[0]);
    client->reg();
    sendRWelcome(client);
}

void Command::handlePass(Client* client, const std::vector<std::string>& args)
{
    if (client->isAuthenticated())
        return ;
    if (client->isRegistered())
    {
        sendError(client, ERR_ALREADYREGISTRED);
    }
    if (args.back() != _server.getPassword())
    {
        sendError(client, ERR_PASSWDMISMATCH);
        return ;
    }
    client->authenticate();
    return ;
}

void Command::execute(Client *client, const std::string &line)
{
    std::istringstream stream(line);
    std::string cmd, token;
    std::vector<std::string> args;

    stream >> cmd;
    std::cout << "Command: " << cmd << std::endl;
    while (stream >> token)
    {
        if (token[0] == ':')
        {
            std::string trailingArg = token.substr(1);
            std::string rest;
            std::getline(stream, rest);
            trailingArg += rest;
            args.push_back(trailingArg);
            break;            
        }
        args.push_back(token);
    }

    if (args.empty())
    {
        sendError(client, ERR_NEEDMOREPARAMS);
        return ;
    }

    for(size_t  i = 0; i < args.size(); i++)
    {
        std::cout << args[i] << std::endl;
    }

    std::map<std::string, CommandHandler>::iterator it = _commands.find(cmd);

    if (it != _commands.end())
    {
        CommandHandler handler = it->second;
        (this->*handler)(client, args);
        if (!client->isAuthenticated())
        {
            epoll_ctl(_server.getEpoll(), EPOLL_CTL_DEL, client->getSocket(), 0);
            close(client->getSocket());
            _server.getClients().erase(client->getSocket());
        }
    }
    else
    {
        sendError(client, ERR_UNKNOWNCOMMAND);
        // std::string msg = ":ircserver " + cmd + " * 421 :Unknown command\r\n";
        // std::string msg = ":ircserver 421 * :Unknown command\r\n";
        // std::cout << msg << std::endl;
        // send(client->getSocket(), msg.c_str(), msg.size(), 0);

        // send(client->getSocket(), "Unkown command", 15, 0);
        // sendError();
    }
}