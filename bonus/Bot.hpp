#ifndef BOT_HPP
#define BOT_HPP

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#pragma once

class Bot
{
public:
    Bot(int socket);
    ~Bot();
    void write(const std::string& message);
    void execute(const std::string& line);
private:
    int _socket;
};

#endif