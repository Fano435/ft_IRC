#include "Client.hpp"
#include "server.h"
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>

Client::Client(const int sock, struct sockaddr_in &addr, socklen_t addr_len) :  _socket(sock), _authenticated(false)
{
    char host[NI_MAXHOST];

    if (getnameinfo((struct sockaddr*)&addr, addr_len, host, sizeof(host), NULL,0, NI_NAMEREQD) == 0)
        _host = std::string(host);
}

Client::Client() : _authenticated(false)
{
}

Client::~Client(){}

int Client::createSocket(const int server_sock) const
{
    (void)server_sock;
    return 0;
}

int Client::getSocket() const
{
    return _socket;
}

void Client::setNickname(const std::string &name)
{
    _nickname = name;
}

std::string Client::getNickname() const
{
    return _nickname;
}

std::string Client::getHost() const
{
    return _host;
}

std::string Client::getUsername() const
{
    return _username;
}

void Client::setUsername(const std::string &name)
{
    _username = name;
}

bool Client::isAuthenticated() const
{
    return _authenticated;
}

void Client::authenticate() 
{
    _authenticated = true;
}

bool Client::isRegistered() const
{
    return _registered;
}

void Client::reg() 
{
    _registered = true;
}