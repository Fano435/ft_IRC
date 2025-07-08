#include "Bot.hpp"

Bot::Bot(int socket) : _socket(socket)
{

}

Bot::~Bot()
{

}

void Bot::write(const std::string& message)
{
    std::string buffer = message + "\r\n";
    if (send(_socket, buffer.c_str(), buffer.length(), 0) < 0)
        throw std::runtime_error("Error while sending a message to a client!");
}

void Bot::execute(const std::string& line) {
    std::cout << "<< " << line << std::endl;

    if (line.find("PING") == 0) {
        std::string pong = "PONG" + line.substr(4) + "\r\n";
        send(_socket, pong.c_str(), pong.size(), 0);
        return;
    }

    if (line.find("PRIVMSG") != std::string::npos && line.find("!hello") != std::string::npos) {
        size_t start = line.find("PRIVMSG");
        size_t channel_start = line.find("#", start);
        size_t colon = line.find(":", channel_start);

        std::string channel = line.substr(channel_start, colon - channel_start);
        std::string response = "PRIVMSG " + channel + " :salut\r\n";
        send(_socket, response.c_str(), response.length(), 0);
    }
}
