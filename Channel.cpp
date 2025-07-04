#include "Channel.hpp"

Channel::Channel(Server &server, Client* admin, const std::string name) : _server(server), _name(name)
{
    (void)_l;
    _admins.insert(admin);
    addClient(admin, name);
}

// void Channel::setAdmin(Client* client)
// {
//     _admins.insert(client);
// }

void Channel::addClient(Client* client, std::string name)
{
    if (!client)
        return ;
    _clients[client->getNickname()] = client;
    broadcast(client, "JOIN " + name);
}

void Channel::removeClient(Client* client, const std::string &msg)
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
}

bool mode_isvalid(char c)
{
    static const std::string valid_modes = "itkol";
    return valid_modes.find(c) != std::string::npos;
}

// void Channel::change_priv(Client *client, std::string nick)
// {

// }

void Channel::setMode(Client *client, const std::vector<std::string>& args)
{
    bool adding = true;
    std::string modestring = args[1];

    if (_admins.find(client) == _admins.end())
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
            return;
        }
        if (adding)
        _modes.insert(c);
        else
        _modes.erase(c);
    }
    // _server.reply(client, "MODE " + _name + " :" + modestring);
}

std::string Channel::getMode() const 
{
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
