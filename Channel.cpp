#include "Channel.hpp"

Channel::Channel(Client* admin, const std::string name) : _admin(admin), _name(name)
{
    (void)_l;
    addClient(admin, name);
}

void Channel::addClient(Client* client, std::string name)
{
    if (!client)
        return ;
    _clients[client->getSocket()] = client;
    broadcast(client, "JOIN " + name);
}

void Channel::removeClient(Client* client, const std::string &msg)
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
        if (user == _admin)
            list += "@";
        list += user->getNickname();
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
        if (it->first != exclude_fd)
            it->second->write(client->getPrefix() + " " + message);
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

std::time_t Channel::get_topic_time() const
{
    return _topic_time;
}

Channel::~Channel()
{

}
bool Channel::client_in_channel(Client* client)
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++ )
    {
        if (it->second == client)
         return true;
    }
    return false;
}

bool Channel::client_in_channel_str(std::string client_target)
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second && it->second->getNickname() == client_target)
            return true;
    }
    return false;
}