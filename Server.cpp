#include "Server.hpp"
#include "server.h"
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

Server::Server(const char *s_port, const std::string password) : _password(password), _server("ircserver")
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
    initReplies();

    _commands["CAP"] = &Server::handleCap;
    _commands["PASS"] = &Server::handlePass;
    _commands["NICK"] = &Server::handleNick;
    _commands["USER"] = &Server::handleUser;
}

void Server::sendError(Client* client, int code, const std::string& param = "") {
    std::ostringstream oss;
    std::string target = client->getNickname().empty() ? "*" : client->getNickname();

    oss << ":" << _server << " "
        << std::setw(3) << std::setfill('0') << code << " "
        << target;

    if (!param.empty())
        oss << " " << param;

    oss << " :" << _errors[code] << "\r\n";

    std::string reply = oss.str();
    send(client->getSocket(), reply.c_str(), reply.length(), 0);
}

void Server::sendRWelcome(Client* client)
{
    std::ostringstream oss;
    std::string target = client->getNickname().empty() ? "*" : client->getNickname();

    oss << ":" << _server << " "
        << std::setw(3) << std::setfill('0') << RPL_WELCOME << " " << target << " :"
        << "Welcome to the " << _server << " Network, " << client->getNickname();
    
    std::string reply = oss.str();
    send(client->getSocket(), reply.c_str(), reply.length(), 0);
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

void Server::handleNick(Client* client, const std::vector<std::string>& args)
{
    if (!client->isAuthenticated())
        return ;
    std::string nick = args[0];
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++)
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

void Server::handleCap(Client* client, const std::vector<std::string>& args)
{
    (void)client;
    (void)args;
}

void Server::handleUser(Client* client, const std::vector<std::string>& args)
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

void Server::handlePass(Client* client, const std::vector<std::string>& args)
{
    if (client->isAuthenticated())
        return ;
    if (client->isRegistered())
    {
        sendError(client, ERR_ALREADYREGISTRED);
    }
    if (args.back() != _password)
    {
        sendError(client, ERR_PASSWDMISMATCH);
        return ;
    }
    client->authenticate();
    return ;
}

void Server::disconnect(Client* client, const std::string& reason)
{

}

void Server::parseLine(Client *client, const std::string &line)
{
    (void)client;
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
            epoll_ctl(epoll_fd)
            close(client->getSocket());
            _clients.erase(client->getSocket());
            // delete _clients[client->getSocket()];
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

void Server::parseMsg(int sender_sock)
{
    char buffer[BUFFER_SIZE];
    memset(&buffer, '\0', sizeof buffer);
    int bytes_read = recv(sender_sock, buffer, BUFFER_SIZE, 0);
    if ( bytes_read <= 0)
    {
        if (bytes_read == -1)
            throw std::runtime_error("[Server] Error: recv");
        std::cout << "Client socket " << sender_sock << " closed" << std::endl;
        close(sender_sock);
    }
    else
    {
        std::string recvBuffer = buffer;
        size_t pos = 0;
        while ((pos = recvBuffer.find("\r\n")) != std::string::npos)
        {
            std::string line = recvBuffer.substr(0, pos);
            recvBuffer.erase(0, pos + 2);
            parseLine(_clients[sender_sock], line);
        }
        // std::cout << buffer << std::endl;
    }
}

void Server::run()
{
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
                _clients[client_sock] = new Client(client_sock);
            }
            else
            {
                /*Ce n'est pas une nouvelle connexion donc il s'agit de gerer un socket existant (lire/ecrire)*/
                parseMsg(events[n].data.fd);
            }
        }
    }
}
