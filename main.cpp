#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <cstring>
#include "Server.hpp"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        return 0;
    }
    try
    {
        Server server(av[1], std::string(av[2]));

        server.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}