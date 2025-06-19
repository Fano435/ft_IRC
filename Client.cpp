#include "Client.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>

Client::Client(const int sock) :  _socket(sock), _authenticated(false)
{
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