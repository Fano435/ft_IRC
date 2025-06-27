#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <sys/socket.h>

class Client
{
private:
    int _socket;
    std::string _username;
    std::string _nickname;
    std::string _realname;
    std::string _host;

    bool _authenticated;
    bool _registered;

public:
    Client();
    Client(const int sock, struct sockaddr_in &addr, socklen_t addr_len);
    ~Client();
    int getSocket() const;
    int createSocket(const int server_sock) const;
    void setNickname(const std::string &name);
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getHost() const;
    void setUsername(const std::string &name);
    bool isAuthenticated() const;
    void authenticate();
    void reg();
    bool isRegistered() const;
    void run();
};


#endif