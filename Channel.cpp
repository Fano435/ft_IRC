#include "Channel.hpp"
#include <cstdlib>

Channel::Channel(Server &server, Client* admin, const std::string name) : _server(server), _name(name), _i(false), _t(false)
{
    (void)_l;
    (void)_i;
    (void)_t;
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

void Channel::add(Client* client)
{
    if (!client)
        return ;
    _clients[client->getNickname()] = client;
    broadcast(client, "JOIN " + _name);
}

void Channel::remove(Client* client, const std::string &msg)
{
    std::map<std::string, Client*>::iterator it = _clients.find(client->getNickname());
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
    std::string user = args[1];
    std::map<std::string, Client*>::iterator it = _clients.find(user);
    if (it != _clients.end())
    {
        std::string message = "KICK " + _name + " " + user + " :" + args[2];
        broadcast(client, message);
        _clients.erase(it);
    }
    else
    {
        sendError(client, ERR_USERNOTINCHANNEL, user + " " + _name);
    }
}

bool Channel::has_client(Client *client)
{
    return (_clients.find(client->getNickname()) != _clients.end());
}

bool Channel::is_admin(Client *client)
{
    return (_admins.find(client) != _admins.end());
}

bool mode_isvalid(char c)
{
    static const std::string valid_modes = "itkol";
    return valid_modes.find(c) != std::string::npos;
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
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
            if (_clients.find(nick) != _clients.end())
            {
                if (adding)
                    _admins.insert(_clients[nick]);
                else
                    _admins.erase(_clients[nick]);
                broadcast(client, "MODE " + _name + " " + modestring + " " + nick);
            }
            else
                sendError(client, ERR_USERNOTINCHANNEL, nick + " " + _name);
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
            }
            else
            {
                _l = 0;
                _modes.erase('l');
            }
            broadcast(client, "MODE " + _name + " " + modestring + " " + args[2]);
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
        else if (c == 't')
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
    for (std::map<std::string, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        Client* user = it->second;
        if (it != _clients.begin())
        {
            list += " ";
        }
        if (_admins.find(user) != _admins.end())
            list += "@";
        list += it->first;
    }
    return list;
}

void Channel::broadcast(Client* client, const std::string &message, int exclude_fd)
{
    if (_clients.find(client->getNickname()) == _clients.end())
    {
        sendError(client, ERR_NOTONCHANNEL, _name);
        return ;
    }
    for (std::map<std::string, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        Client* user = it->second;
        if (user->getSocket() != exclude_fd)
            user->write(client->getPrefix() + " " + message);
    }
}

void Channel::setTopic(const std::string topic)
{
    _topic = topic;
}

std::string Channel::getTopic() const
{
    return _topic;
}

Channel::~Channel()
{

}
