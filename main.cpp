#include "server.h"
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

        initErrors();
        server.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}