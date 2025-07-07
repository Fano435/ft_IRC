#include "Client.hpp"
#include "server.h"
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>

Client::Client(const int sock, struct sockaddr_in &addr, socklen_t addr_len) : _socket(sock), _username("*"), _nickname("*"),  _authenticated(false), _registered(false)
{
    char host[NI_MAXHOST];

    if (getnameinfo((struct sockaddr*)&addr, addr_len, host, NI_MAXHOST, NULL,0, NI_NUMERICSERV) == 0)
        _hostname = host;
    else
        throw std::runtime_error("Error while getting a hostname on a new client!");
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

void Client::append_buf(const std::string& buffer)
{
    _buffer += buffer;
}

std::string &Client::getBuffer()
{
    return _buffer;
}


std::string Client::getPrefix() const
{
    std::string prefix = ":" + getNickname() + "!" + _username + "@" + _hostname;
    return prefix;
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

void Client::welcome() 
{
    _registered = true;
    reply(RPL_WELCOME(_nickname));
}


void Client::write(const std::string& message)
{
    std::string buffer = message + "\r\n";
    if (send(_socket, buffer.c_str(), buffer.length(), 0) < 0)
        throw std::runtime_error("Error while sending a message to a client!");
}

void Client::reply(const std::string& reply)
{
    write(getPrefix() + " " + reply);
}