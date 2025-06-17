#include "Server.hpp"
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
#include <iostream>
#include <utility>
#include <sstream>

Server::~Server()
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        delete (*it).second;
    }
}

Server::Server(const char *s_port, const std::string password) : _password(password)
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
    initializeErrorMessages();

    _commands["NICK"] = &Server::handleNick;
    _commands["USER"] = &Server::handleUser;
    _commands["PASS"] = &Server::handlePass;
}

void Server::initializeErrorMessages() {
    _errors[ERR_NONICKNAMEGIVEN] = ":No nickname given";
    _errors[ERR_ERRONEUSNICKNAME] = ":Erroneous nickname";
    _errors[ERR_NICKNAMEINUSE] = ":Nickname is already in use";
}

int Server::createSocket() const
{
    struct sockaddr_in server_addr;

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        throw std::runtime_error("Error while creating socket");
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

void Server::handleNick(Client* client, const std::vector<std::string>& args)
{
    if (args.empty())
        return ;
    std::string nick = args[0];
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if ((*it).second->getNickname() == nick)
            return ;
    }
    client->setNickname(args[0]);
}

void Server::handleUser(Client* client, const std::vector<std::string>& args)
{
    client->setUsername(args[0]);
}

void Server::handlePass(Client* client, const std::vector<std::string>& args)
{
    (void)client;
    if (args.empty())
        return ;
    if (args[0] == _password)
    {
        // It's good
    }
    return ;
}

void Server::parseLine(Client *client, const std::string &line)
{
    (void)client;
    std::istringstream stream(line);
    std::string cmd;
    std::vector<std::string> args;

    std::getline(stream, cmd, ' ');

}

void Server::run()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct epoll_event ev, events[MAX_EVENTS];
    int client_sock, nfds, epoll_fd;
    char buffer[BUFFER_SIZE];

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) 
    {
        throw std::runtime_error("Error: epoll_create1");
    }
    ev.events = EPOLLIN;
    ev.data.fd = _socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socket, &ev) == -1 )
    {
        throw std::runtime_error("Error: epoll_ctl: server_sock");
    }

    while(1)
    {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) 
        {
            throw std::runtime_error("Error: epoll_wait");
        }
        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == _socket)
            {
                client_sock = accept4(_socket, (struct sockaddr *)&client_addr, &client_addr_len, SOCK_NONBLOCK);
                if (client_sock == -1)
                {
                    throw std::runtime_error("Error: accept4");
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_sock;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &ev) == -1 )
                {
                    throw std::runtime_error("Error: epoll_ctl: client_sock");
                }
            }
            else
            {
                memset(&buffer, '\0', sizeof buffer);
                int bytes_read = recv(events[n].data.fd, buffer, BUFFER_SIZE, 0);
                if ( bytes_read <= 0)
                {
                    if (bytes_read == -1)
                        throw std::runtime_error("[Server] Error: recv");
                    std::cout << "Client socket " << events[n].data.fd << " closed" << std::endl;
                    close(events[n].data.fd);
                }
                else
                {
                    std::string recvBuffer = buffer;
                    size_t pos = 0;
                    while ((pos = recvBuffer.find("\r\n")) != std::string::npos)
                    {
                        std::string line = recvBuffer.substr(0, pos);
                        recvBuffer.erase(0, pos + 2);

                    }
                    
                    std::cout << buffer ;
                }
                /*Ce n'est pas une nouvelle connexion donc il s'agit de gerer un socket existant (lire/ecrire)*/
            }
        }
    }
}
