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
    _commands["JOIN"] = &Command::join;
    _commands["PART"] = &Command::leave;
    _commands["TOPIC"] = &Command::topic;
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

std::vector<std::string> split(const std::string& input, char delimiter) {
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type end;

    while ((end = input.find(delimiter, start)) != std::string::npos) {
        if (end > start) {
            result.push_back(input.substr(start, end - start));
        }
        start = end + 1;
    }

    if (start < input.size()) {
        result.push_back(input.substr(start));
    }

    return result;
}

void Command::leave(Client* client, const std::vector<std::string>& args)
{
    if (args.empty())
    {
        sendError(client, ERR_NEEDMOREPARAMS, "PART");
        return ;
    }
    std::string reason;
    if (args.size() == 2)
        reason = args[1];
    std::vector<std::string> channels = split(args[0], ',');

    for (std::vector<std::string>::iterator it = channels.begin(); it < channels.end(); it++)
    {
        if ((*it)[0] != '#')
            continue;
        _server.removeFromChannel(client, *it, reason);
    }
}


void Command::join(Client* client, const std::vector<std::string>& args)
{
    std::string param = args[0];
    if (param == "0")
    {
        _server.removeFromAll(client);
        return ;
    }

    std::vector<std::string> channels = split(args[0], ',');
    
    for (std::vector<std::string>::iterator it = channels.begin(); it < channels.end(); it++)
    {
        if ((*it)[0] != '#')
            continue;
        _server.addToChannel(client, *it);
    }
}

void Command::message(Client* client, const std::vector<std::string>& args)
{
    std::string target = args[0];
    std::string msg = args[1];

    if (args.empty())
    {
        sendNumeric(client, ERR_NORECIPIENT, "PRIVMSG");
        return ;
    }
    if (msg.empty())
    {
        sendError(client, ERR_NOTEXTTOSEND);
        return ;
    }
    if (target[0] == '#')
    {
        _server.messageChannel(client, target, msg);
        return ;
    }
    std::string message = client->getPrefix() + " PRIVMSG " + target + " :" + msg + "\r\n";
    for (std::map<int, Client *>::iterator it = _server.getClients().begin(); it != _server.getClients().end(); it++)
    {
        if ((*it).second->getNickname() == target)
        {
            send((*it).first, message.c_str(), message.length(), 0);
            return ;
        }
    }
    sendError(client, ERR_NOSUCHNICK, target);
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
    client->welcome();
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

void Command::topic(Client* client, const std::vector<std::string>& args)
{

    /* to do :
    Clients joining the channel in the future will receive a RPL_TOPIC numeric (or lack thereof) accordingly.
    
    les perm / mode ERR

    [#vovo] /topic ytr  (ca change le topic. c est sense marcher ?)
    */

    std::string channel_name = args[0];
    if (channel_name[0] != '#') {
        sendError(client, ERR_NOSUCHCHANNEL, channel_name);
        return;
    }


    std::map<std::string, Channel *>::iterator it = _server.getChannels().find(channel_name);
    if (it == _server.getChannels().end()) { // le channel existe ou pas ?
        sendError(client, ERR_NOSUCHCHANNEL, channel_name);
        return;
    }
    Channel* channel = it->second;

    if (!channel->client_in_channel(client)) // si le client est pas dans le channel 
    {
        sendError(client, ERR_NOTONCHANNEL,channel_name);
        return ;
    }

    // Si pas de topic donné, renvoyer le topic actuel
    if (args.size() == 1 ) {
        _server.reply(client, RPL_TOPIC(client->getNickname(), channel_name, channel->getTopic()));
        std::ostringstream oss;
        oss << channel->get_topic_time();
        _server.reply(client, RPL_TOPICWHOTIME(client->getNickname(), channel_name, client->getNickname(), oss.str()));
        return;
    }


    
    std::string new_topic = args[1];
    channel->setTopic(new_topic);
    channel->broadcast(client, "TOPIC " + channel_name + " :" + new_topic);
}