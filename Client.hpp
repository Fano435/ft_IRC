#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <sys/socket.h>
#include <set>

class Client
{
public:
    int _socket;
    std::string _username;
    std::string _nickname;
    std::string _realname;
    std::string _hostname;

    bool _authenticated;
    bool _registered;
    int createSocket(const int server_sock) const;

    std::string _buffer;

public:
    Client();
    Client(const int sock, struct sockaddr_in &addr, socklen_t addr_len);
    ~Client();
    int getSocket() const;
    void setNickname(const std::string &name);
    std::string getNickname() const;
    std::string getPrefix() const;
    void append_buf(const std::string& buffer);
    std::string &getBuffer();
    void setUsername(const std::string &name);
    bool isAuthenticated() const;
    void authenticate(const bool status);
    void welcome();
    bool isRegistered() const;
    void write(const std::string& message);
    void reply(const std::string& reply);
};


#endif