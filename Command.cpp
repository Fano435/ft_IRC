#include "Command.hpp"
#include "Client.hpp"
#include "server.h"
#include <sstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

Command::Command(Server &server) : _server(server)
{
    _commands["CAP"] = &Command::ignore;
    _commands["PASS"] = &Command::handlePass;
    _commands["NICK"] = &Command::handleNick;
    _commands["USER"] = &Command::handleUser;
    _commands["QUIT"] = &Command::quit;
    _commands["PING"] = &Command::ping;
    _commands["PRIVMSG"] = &Command::handleMessage;
    _commands["NOTICE"] = &Command::handleNotice;
    _commands["JOIN"] = &Command::join;
    _commands["PART"] = &Command::part;
    _commands["MODE"] = &Command::handleMode;
    _commands["KICK"] = &Command::handleKick;
    _commands["WHO"] = &Command::ignore;
    _commands["WHOIS"] = &Command::ignore;
    _commands["PART"] = &Command::part;
    _commands["TOPIC"] = &Command::topic;
    _commands["INVITE"] = &Command::handleInvite;
}

Command::~Command()
{

}

void Command::part(Client* client, const std::vector<std::string>& args)
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
        Channel *channel = _server.getChannel(*it);
        if (!channel)
        {
            sendError(client, ERR_NOSUCHCHANNEL, *it);
            return ;
        }
        channel->remove(client, reason);
    }
}

void Command::handleKick(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        sendError(client, ERR_NEEDMOREPARAMS, "KICK");
        return ;
    }

    Channel *channel = _server.getChannel(args[0]);
    if (!channel)
    {
        sendError(client, ERR_NOSUCHCHANNEL, args[0]);
        return ;
    }
    channel->kick(client, args);
}

void Command::handleMode(Client* client, const std::vector<std::string>& args)
{
    std::string target = args[0];

    if(target[0] != '#')
        return ;
    Channel *channel = _server.getChannel(target);
    if (!channel)
    {
        sendError(client, ERR_NOSUCHCHANNEL, target);
        return ;
    }
    if (args.size() < 2)
    {
        _server.reply(client, RPL_CHANNELMODEIS(client->getNickname(), target, channel->getMode()));
        return ;
    }
    else
        channel->setMode(client, args);
}

void Command::join(Client* client, const std::vector<std::string>& args)
{
    std::string param = args[0];
    std::vector<std::string> keys;
    if (param == "0")
    {
        _server.removeFromAll(client);
        return ;
    }

    std::vector<std::string> channels = split(args[0], ',');
    if (args.size() > 1)
        keys = split(args[1], ',');
    for (size_t i = 0; i < channels.size(); i++)
    {
        if (channels[i][0] != '#')
            continue;
        Channel *channel = _server.getChannel(channels[i]);

        std::string key = (i < keys.size()) ? keys[i] : "";
        if (channel && !channel->check_key(key))
        {
            _server.reply(client, ERR_BADCHANNELKEY(client->getNickname(), channels[i]));
            return ;
        }
        _server.addToChannel(client, channels[i]);
    }  
}

void Command::handleNotice(Client* client, const std::vector<std::string>& args)
{
    std::string msg = "";
    if (args.size() >= 2)
        msg = args[1];

    std::vector<std::string> targets = split(args[0], ',');
    for (std::vector<std::string>::iterator target = targets.begin(); target < targets.end(); target++)
    {
        std::string message = "NOTICE " + *target + " :" + msg + "\r\n";
        if ((*target)[0] != '#')
        {
            Client *receiver = _server.getClient(*target);
            if (receiver)
                send(receiver->getSocket(), message.c_str(), message.length(), 0);
        }
        else
        {
            Channel *channel = _server.getChannel(*target);
            if (channel && channel->is_admin(client))
            {
                channel->broadcast(client, message);
            }
        }
    }
}

void Command::handleMessage(Client* client, const std::vector<std::string>& args)
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
    Client *receiver = _server.getClient(target);
    if (!receiver)
    {
        sendError(client, ERR_NOSUCHNICK, target);
        return ;
    }
    send(receiver->getSocket(), message.c_str(), message.length(), 0);
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
    send(client->getSocket(), reply.c_str(), reply.size(), 0);
}

void Command::handleNick(Client* client, const std::vector<std::string>& args)
{
    if (!client->isAuthenticated())
        return ;
    std::string nick = args[0];
    Client *in_use = _server.getClient(nick);
    if (in_use)
    {
        sendError(client, ERR_NICKNAMEINUSE, nick);
        if (!client->isRegistered())
        {
            client->authenticate(false);
        }
        return ;
    }
    if(!isValidNickname(nick))
    {
        sendError(client, ERR_ERRONEUSNICKNAME);
        client->authenticate(false);
        return ;
    }
    std::string message = "NICK " + nick;
    if (client->isRegistered())
    {
        _server.replyToAll(client, message);
    }
    client->setNickname(nick);
}

void Command::ignore(Client* client, const std::vector<std::string>& args)
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
    client->authenticate(true);
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
    std::string channel_name = args[0];
    Channel *channel = _server.getChannel(channel_name);
    if (channel_name[0] != '#' || !channel) {
        sendError(client, ERR_NOSUCHCHANNEL, channel_name);
        return;
    }

    if (!channel->has_client(client)) // si le client est pas dans le channel 
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
    channel->setTopic(client, new_topic);
}

void Command::handleInvite(Client* client, const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        sendError(client, ERR_NEEDMOREPARAMS, "INVITE");
        return ;
    }

    // Parameters: <nickname> <channel>
    std::string nickname_target = args[0];
    std::string channel_name = args[1];

    Channel *channel = _server.getChannel(channel_name);
    if (channel_name[0] != '#' || !channel)
    {
        sendError(client, ERR_NOSUCHCHANNEL, channel_name);
        return ;
    }
    if (!channel->has_client(client))
    {
        sendError(client, ERR_NOTONCHANNEL,channel_name);
        return ;
    }
    if (channel->getClient(nickname_target))
    {
        _server.reply(client, ERR_USERONCHANNEL(client->getNickname(), nickname_target,channel_name ) );
        return ;
    }
    
    channel->invite(client, nickname_target);
}