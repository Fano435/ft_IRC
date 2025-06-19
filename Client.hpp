#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client
{
private:
    int _socket;
    std::string _username;
    std::string _nickname;
    std::string _realname;
    bool _authenticated;
    bool _registered;

public:
    Client();
    Client(const int sock);
    ~Client();
    int getSocket() const;
    int createSocket(const int server_sock) const;
    void setNickname(const std::string &name);
    std::string getNickname() const;
    void setUsername(const std::string &name);
    bool isAuthenticated() const;
    void authenticate();
    void reg();
    bool isRegistered() const;
    void run();
};


#endif