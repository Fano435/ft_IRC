#include "Server.hpp"
#include "server.h"
#include "Command.hpp"
#include <unistd.h>
#include <cstdlib>
#include <cctype>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <utility>
#include <sstream>
#include <iomanip>

Server::~Server()
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        delete (*it).second;
    }
}

Server::Server(const char *s_port, const std::string password) : _password(password), _server(SERVER)
{
    int i = 0;
    while (s_port[i])
    {
        if (!isdigit(s_port[i++]))
            throw std::invalid_argument("Error : invalid port");
    }
    _port = atoi(s_port);
    if (_port < 0 || _port > MAX_PORT)
        throw std::invalid_argument("Error : invalid port");

    _socket = createSocket();
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) 
        throw std::runtime_error("Error: epoll_create1");

}
std::map<int, Client *> &Server::getClients()
{
    return _clients;
}

const std::string Server::getPassword() const
{
    return _password;
}


int Server::createSocket() const
{
    struct sockaddr_in server_addr;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        throw std::runtime_error("Error while creating socket");
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Error while binding socket");

    if (listen(server_sock, MAX_EVENTS))
        throw std::runtime_error("Error while listening socket");

    std::cout << "Server listening on port " << _port << std::endl;
    return server_sock;
}

void Server::parseMsg(int sender_sock)
{
    Client *client = _clients[sender_sock];
    char buffer[BUFFER_SIZE];
    memset(&buffer, '\0', sizeof buffer);
    int bytes_read = recv(sender_sock, buffer, BUFFER_SIZE, 0);
    if (bytes_read <= 0)
    {
        if (bytes_read == -1)
            throw std::runtime_error("[Server] Error: recv");
        std::cout << "Client socket " << sender_sock << " closed" << std::endl;
        disconnect(client, "");
    }
    else
    {
        client->append_buf(buffer);
        std::string& buf = client->getBuffer();
        size_t pos = 0;
        while ((pos = buf.find("\r\n")) != std::string::npos)
        {
            std::string line = buf.substr(0, pos);
            std::cout << line << std::endl;
            buf.erase(0, pos + 2);
            _command->execute(client, line);
        }
        // std::cout << buffer << std::endl;
    }
}

void Server::run()
{
    _command = new Command(*this);
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct epoll_event ev, events[MAX_EVENTS];
    int client_sock, nfds;

    ev.events = EPOLLIN;
    ev.data.fd = _socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socket, &ev) == -1 )
        throw std::runtime_error("Error: epoll_ctl: server_sock");

    while(1)
    {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1)
            throw std::runtime_error("Error: epoll_wait");
        for (int n = 0; n < nfds; ++n)
        {
            // Si premiere connexion, ajout d'un nouveau socket client
            if (events[n].data.fd == _socket)
            {
                client_sock = accept4(_socket, (struct sockaddr *)&client_addr, &client_addr_len, SOCK_NONBLOCK);
                if (client_sock == -1)
                    throw std::runtime_error("Error: accept4");
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_sock;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &ev) == -1 )
                    throw std::runtime_error("Error: epoll_ctl: client_sock");
                _clients[client_sock] = new Client(client_sock, client_addr, client_addr_len);
            }
            else
            {
                // std::cout << "Clients before: " << getClients().size() << std::endl;
                parseMsg(events[n].data.fd);
                // std::cout << "Clients after: " << getClients().size() << std::endl;
            }
        }
    }
}

void Server::disconnect(Client* client, const std::string& reason)
{
    if(!reason.empty())
    {
        std::string msg = "ERROR :" + reason + "\r\n";
        send(client->getSocket(), msg.c_str(), msg.length(), 0);
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->getSocket(), 0);
    close(client->getSocket());

    std::map<int, Client*>::iterator it = _clients.find(client->getSocket());
    if (it != _clients.end()) {
        delete it->second;
        _clients.erase(it);
    }
}

void Server::reply(Client *receiver, const std::string& reply)
{
    std::ostringstream oss;
    oss << ":" << SERVER << " " << reply << "\r\n";

    std::string msg = oss.str();
    send(receiver->getSocket(), msg.c_str(), msg.length(), 0);
}

void Server::addToChannel(Client *client, std::string name)
{
    if (_channels.find(name) != _channels.end())
        _channels[name]->addClient(client, name);
    else
        _channels[name] = new Channel(*this, client, name);
    Channel *chan = _channels[name];
    if (!chan->getTopic().empty())
        reply(client, RPL_TOPIC(client->getNickname(), name, chan->getTopic()));
    reply(client, RPL_NAMREPLY(client->getNickname(), "=", name) + chan->listUsers());
    reply(client, RPL_ENDOFNAMES(client->getNickname(), name));
}

void Server::removeFromChannel(Client *client, std::string name, const std::string &msg)
{
    if (_channels.find(name) == _channels.end())
    {
        sendError(client, ERR_NOSUCHCHANNEL, name);
        return ;
    }
    _channels[name]->removeClient(client, msg);
}

void Server::changeMode(Client *client, const std::vector<std::string>& args)
{
    std::string target = args[0];

    if(target[0] == '#')
    {
        if (_channels.find(target) == _channels.end())
        {
            sendError(client, ERR_NOSUCHCHANNEL, target);
            return ;
        }
        if (args.size() < 2)
        {
            reply(client, RPL_CHANNELMODEIS(client->getNickname(), target, _channels[target]->getMode()));
            return ;
        }
        else
        {
            _channels[target]->setMode(client, args);
            return ;
        }
    }
}

void Server::removeFromAll(Client *client)
{
    for (chan_iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        it->second->removeClient(client, "");
    }
}

void Server::messageChannel(Client *client, std::string name, const std::string &msg)
{
    if (_channels.find(name) == _channels.end())
    {
        sendError(client, ERR_NOSUCHCHANNEL, name);
        return ;
    }
    std::string message = "PRIVMSG " + name + " :" + msg;
    _channels[name]->broadcast(client, message, client->getSocket());
}
