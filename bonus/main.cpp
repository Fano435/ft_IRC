#include "Bot.hpp"
#include "../Server.hpp"
#include <cstdlib>
#include <cstring>

int main(int ac, char **av)
{
    if (ac != 3)
        return 0;
    int port = atoi(av[1]);
    std::string password = av[2];

    int bot_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    connect(bot_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    Bot bot(bot_socket);

    bot.write("PASS " + password);
    bot.write("NICK BOT" );
    bot.write("USER CHATBOT 0 * :CHATBOT");
    bot.write("JOIN #chat");

    char buffer[BUFFER_SIZE];
    while (true)
    {
        memset(&buffer, '\0', sizeof buffer);
        std::string buf;
        size_t pos = 0;
        int bytes_read = recv(bot_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0)
            return 1;
        else
            buf += buffer;
        while ((pos = buf.find("\r\n")) != std::string::npos)
        {
            std::string line = buf.substr(0, pos);
            std::cout << line << std::endl;
            buf.erase(0, pos + 2);
    
            bot.execute(line);
        }
    }
    
    close(bot_socket);
    return 0;
}



