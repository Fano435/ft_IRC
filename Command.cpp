#include "Command.hpp"
#include "Client.hpp"
#include "server.h"
#include <sstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Command::Command(Server &server) : _server(server)
{
    _commands["CAP"] = &Command::cap;
    _commands["PASS"] = &Command::handlePass;
    _commands["NICK"] = &Command::handleNick;
    _commands["USER"] = &Command::handleUser;
    _commands["QUIT"] = &Command::quit;
    _commands["PING"] = &Command::ping;
    _commands["PRIVMSG"] = &Command::message;
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

void Command::message(Client* client, const std::vector<std::string>& args)
{
    std::string target = args[0];
    std::string msg = args[1];

    if (args.empty())
    {
        sendError(client, ERR_NORECIPIENT);
        return ;
    }
    std::string message = ":" + client->getNickname() + "!" + client->getUsername()
    + "@" + client->getHost() + " PRIVMSG " + target + " :" + msg + "\r\n";
    for (std::map<int, Client *>::iterator it = _server.getClients().begin(); it != _server.getClients().end(); it++)
    {
        if ((*it).second->getNickname() == target)
        {
            // std::cout << "sender fd: " << client->getSocket() << std::endl;
            // std::cout << "receiver fd: " << (*it).first << std::endl;
            send((*it).first, message.c_str(), message.length(), 0);
            return ;
        }
    }
}

void Command::quit(Client* client, const std::vector<std::string>& args)
{
    std::string reason = args[0];
    _server.disconnect(client, reason);
}

void Command::ping(Client* client, const std::vector<std::string>& args) 
{
    if (args.empty()) {
        sendError(client, ERR_NOORIGIN);
        return;
    }

    std::string reply = "PONG :" + args[0] + "\r\n";
    std::cout << args[0] << std::endl;
    send(client->getSocket(), reply.c_str(), reply.size(), 0);
}

void Command::handleNick(Client* client, const std::vector<std::string>& args)
{
    if (!client->isAuthenticated())
        return ;
    std::string nick = args[0];
    // std::map<int, Client *> clients = _server.getClients();
    for (std::map<int, Client *>::iterator it = _server.getClients().begin(); it != _server.getClients().end(); it++)
    {
        if ((*it).second->getNickname() == nick)
        {
            sendError(client, ERR_NICKNAMEINUSE, nick);
            // if ((*it).second->getSocket() != client->getSocket())
            //     _server.disconnect(client, "");
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

void Command::cap(Client* client, const std::vector<std::string>& args)
{
    (void)client;
    (void)args;
}

void Command::handleUser(Client* client, const std::vector<std::string>& args)
{
    if (!client->isAuthenticated())
    {
        _server.disconnect(client, "Authentication failed");
        return ;
    }
    if (client->isRegistered())
    {
        sendError(client, ERR_ALREADYREGISTRED);
        _server.disconnect(client, "");
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
        _server.disconnect(client, "");
        return ;
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

    if (!client)
        return ;
    stream >> cmd;
    // std::cout << "Command: " << cmd << std::endl;
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

    // for(size_t  i = 0; i < args.size(); i++)
    // {
    //     std::cout << args[i] << std::endl;
    // }

    std::map<std::string, CommandHandler>::iterator it = _commands.find(cmd);

    if (it != _commands.end())
    {
        CommandHandler handler = it->second;
        (this->*handler)(client, args);
    }
    else
    {
        sendError(client, ERR_UNKNOWNCOMMAND);
    }
}

