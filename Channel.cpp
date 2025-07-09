#include "Channel.hpp"
#include <cstdlib>

Channel::Channel(Server &server, Client* admin, const std::string name) : _server(server), _name(name), _i(false), _t(false), _l(false)
{
    _admins.insert(admin);
    add(admin);
}

bool Channel::is_full()
{
    return (_l && _clients.size() >= _l);
}

bool Channel::check_key(const std::string &key)
{
    if (_k.empty())
        return true;
    else
        return (_k == key);
}

bool Channel::has_client(Client *client)
{
    return (_clients.find(client->getSocket()) != _clients.end());
}

Client *Channel::getClient(const std::string &name)
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) 
    {
        if (it->second->getNickname() == name) 
            return it->second;
    }
    return NULL;
}

bool Channel::is_admin(Client *client)
{
    return (_admins.find(client) != _admins.end());
}

void Channel::add(Client* client)
{
    if (_i && (_invited.find(client) == _invited.end()))
    {
        _server.reply(client, ERR_INVITEONLYCHAN(client->getNickname(), _name));
        return ;
    }
    _clients[client->getSocket()] = client;
    broadcast(client, "JOIN " + _name);
    if (!getTopic().empty())
        _server.reply(client, RPL_TOPIC(client->getNickname(), _name, _topic));
    _server.reply(client, RPL_NAMREPLY(client->getNickname(), "=", _name) + listUsers());
    _server.reply(client, RPL_ENDOFNAMES(client->getNickname(), _name));
}

void Channel::remove(Client* client, const std::string &msg)
{
    std::map<int, Client*>::iterator it = _clients.find(client->getSocket());
    if (it != _clients.end())
    {
        std::string message = "PART " + _name;
        if (!msg.empty())
            message += " :" + msg;
        broadcast(client, message);
        _clients.erase(it);
    }
    else
    {
        sendError(client, ERR_USERNOTINCHANNEL, client->getNickname() + " " + _name);
    }
}

void Channel::kick(Client *client, const std::vector<std::string>& args)
{
    if (_admins.find(client) == _admins.end())
    {
        _server.reply(client, ERR_CHANOPIVSNEEDED(client->getNickname(), _name));
        return ;
    }
    std::string username = args[1];
    Client *user = getClient(username);
    if (!user)
    {
        sendError(client, ERR_USERNOTINCHANNEL, username + " " + _name);
        return;
    }
    std::string message = "KICK " + _name + " " + username + " :" + args[2];
    broadcast(client, message);
    _clients.erase(user->getSocket());
}

void Channel::setMode(Client *client, const std::vector<std::string>& args)
{
    bool adding = true;
    std::string modestring = args[1];

    if (!is_admin(client))
    {
        _server.reply(client, ERR_CHANOPIVSNEEDED(client->getNickname(), _name));
        return ;
    }
    for (size_t i = 0; i < modestring.size(); ++i)
    {
        char c = modestring[i];

        if (c == '+') 
        {
            adding = true;
            continue;
        } 
        else if (c == '-') 
        {
            adding = false;
            continue;
        }

        if (!mode_isvalid(c))
        {
            _server.reply(client, ERR_UMODEUNKNOWNFLAG(client->getNickname(), c));
            return;
        }
        if (c == 'o') // change privilege
        {
            if (args.size() < 3)
                return ;
            std::string nick = args[2];
            Client *user = getClient(nick);
            if (user)
            {
                if (adding)
                    _admins.insert(user);
                else
                    _admins.erase(user);
                broadcast(client, "MODE " + _name + " " + modestring + " " + nick);
            }
            else
                sendError(client, ERR_USERNOTINCHANNEL, nick + "o " + _name);
            return;
        }
        else if (c == 'l') // change user limit
        {
            if (adding)
            {
                if (args.size() < 3 || !is_number(args[2]))
                {
                    sendError(client, ERR_NEEDMOREPARAMS);
                    return ;
                }
                _l = atoi(args[2].c_str());
                _modes.insert('l');
                broadcast(client, "MODE " + _name + " " + modestring + " " + args[2]);
            }
            else
            {
                _l = 0;
                _modes.erase('l');
                broadcast(client, "MODE " + _name + " " + modestring);
            }
        }
        else if (c == 'k') // change key
        {
            if (args.size() < 3 )
            {
                sendError(client, ERR_NEEDMOREPARAMS);
                return ;
            }
            if (adding)
            {
                _k = args[2];
                _modes.insert('k');
            }
            else
            {
                if (_k != args[2])
                {
                    _server.reply(client, ERR_INVALIDKEY(client->getNickname(), _name));
                    return ;
                }
                _k.clear();
                _modes.erase('k');
            }
            broadcast(client, "MODE " + _name + " " + modestring + " " + args[2]);
        }
        else if (c == 'i') // change invitation only
        {
            if (adding)
            {
                _i = true;
                _modes.insert('i');
            }
            else
            {
                _i = false;
                _modes.erase('i');
            }
            broadcast(client, "MODE " + _name + " " + modestring);
        }
        else if (c == 't') // change topic policy
        {
            if (adding)
            {
                _t = true;
                _modes.insert('t');
            }
            else
            {
                _t = false;
                _modes.erase('t');
            }
            broadcast(client, "MODE " + _name + " " + modestring);
        }
    }
}

std::string Channel::getMode() const 
{
    if (_modes.empty())
        return "";
    std::string modeStr = "+";
    for (std::set<char>::const_iterator it = _modes.begin(); it != _modes.end(); ++it)
        modeStr += *it;
    return modeStr;
}

std::string Channel::listUsers()
{
    std::string list;
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        Client* user = it->second;
        if (it != _clients.begin())
        {
            list += " ";
        }
        if (_admins.find(user) != _admins.end())
            list += "@";
        list += it->second->getNickname();
    }
    return list;
}

void Channel::broadcast(Client* client, const std::string &message, int exclude_fd)
{
    if (_clients.find(client->getSocket()) == _clients.end())
    {
        sendError(client, ERR_NOTONCHANNEL, _name);
        return ;
    }
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        Client* user = it->second;
        if (user->getSocket() != exclude_fd)
            user->write(client->getPrefix() + " " + message);
    }
}

void Channel::setTopic(Client *client, const std::string topic)
{
    if (_t && !is_admin(client))
    {
        _server.reply(client, ERR_CHANOPIVSNEEDED(client->getNickname(), _name));
        return ;
    }
    _topic = topic;
    broadcast(client, "TOPIC " + _name + " :" + topic);
}

std::string Channel::getTopic() const
{
    return _topic;
}

std::time_t Channel::get_topic_time() const
{
    return _topic_time;
}

void Channel::invite(Client *client, const std::string &target_name)
{
    
    Client* target = _server.getClient(target_name);
    if (!target) 
    {
        sendError(client, ERR_NOSUCHNICK, target_name);
        return;
    }
    if (_i)
    {
        if (!is_admin(client))
        {
            _server.reply(client, ERR_CHANOPIVSNEEDED(client->getNickname(), _name));
            return ;
        }
        else
            _invited.insert(target);
    }
    std::string invite_msg = client->getPrefix() + " INVITE " + target_name + " " + _name + "\r\n";
    send(target->getSocket(), invite_msg.c_str(), invite_msg.length(), 0);
    _server.reply(client, RPL_INVITING(client->getNickname(), target_name, _name ));
}

Channel::~Channel()
{

}