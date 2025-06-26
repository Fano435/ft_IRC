#include "errors.hpp"
#include "Server.hpp"
#include <sstream>
#include <sys/socket.h>
#include <iomanip>

std::map<int, std::string> errors;

void initErrors()
{
    errors[ERR_NONICKNAMEGIVEN] = "No nickname given";
    errors[ERR_ERRONEUSNICKNAME] = "Erroneous nickname";
    errors[ERR_NICKNAMEINUSE] = "Nickname is already in use";
    errors[ERR_UNKNOWNCOMMAND] = "Unknown command";
    errors[ERR_NEEDMOREPARAMS] = "Not enough parameters";
    errors[ERR_PASSWDMISMATCH] = "Password incorrect";
    errors[ERR_ALREADYREGISTRED] = "You may not reregister";
}

void sendError(Client* client, int code, const std::string& param = "") 
{
    std::ostringstream oss;
    std::string target = client->getNickname().empty() ? "*" : client->getNickname();

    oss << ":" << SERVER << " "
        << std::setw(3) << std::setfill('0') << code << " "
        << target;

    if (!param.empty())
        oss << " " << param;

    oss << " :" << errors[code] << "\r\n";

    std::string reply = oss.str();
    send(client->getSocket(), reply.c_str(), reply.length(), 0);
}

void sendRWelcome(Client* client)
{
    std::ostringstream oss;
    std::string target = client->getNickname().empty() ? "*" : client->getNickname();

    oss << ":" << SERVER << " "
        << std::setw(3) << std::setfill('0') << RPL_WELCOME << " " << target << " :"
        << "Welcome to the " << SERVER << " Network, " << client->getNickname();
    
    std::string reply = oss.str();
    send(client->getSocket(), reply.c_str(), reply.length(), 0);
}

// void Server::sendError(Client* client, int code, const std::string& param = "") {
//     std::ostringstream oss;
//     std::string target = client->getNickname().empty() ? "*" : client->getNickname();

//     oss << ":" << _server << " "
//         << std::setw(3) << std::setfill('0') << code << " "
//         << target;

//     if (!param.empty())
//         oss << " " << param;

//     oss << " :" << _errors[code] << "\r\n";

//     std::string reply = oss.str();
//     send(client->getSocket(), reply.c_str(), reply.length(), 0);
// }

// void Server::sendRWelcome(Client* client)
// {
//     std::ostringstream oss;
//     std::string target = client->getNickname().empty() ? "*" : client->getNickname();

//     oss << ":" << _server << " "
//         << std::setw(3) << std::setfill('0') << RPL_WELCOME << " " << target << " :"
//         << "Welcome to the " << _server << " Network, " << client->getNickname();
    
//     std::string reply = oss.str();
//     send(client->getSocket(), reply.c_str(), reply.length(), 0);
// }

