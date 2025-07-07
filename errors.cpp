#include "errors.hpp"
#include "Server.hpp"
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iomanip>
#include <unistd.h>

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
    errors[ERR_NOORIGIN] = "No origin specified";
    errors[ERR_NORECIPIENT] = "No recipient given";
    errors[ERR_NOSUCHNICK] = "No such nick/channel";
    errors[ERR_NOSUCHCHANNEL] = "No such channel";
    errors[ERR_NOTEXTTOSEND] = "No text to send";
    errors[ERR_NOTONCHANNEL] = "You're not on that channel";
    errors[ERR_CHANNELISFULL] = "Cannot join channel (+l)";
    errors[ERR_USERNOTINCHANNEL] = "They aren't on that channel";
}

void sendError(Client* client, int code, const std::string& param) 
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

void sendNumeric(Client* client, int code, const std::string& command) {
    std::ostringstream oss;
    oss << ":" << SERVER << " "
        << std::setw(3) << std::setfill('0') << code << " "
        << client->getNickname() << " :No recipient given (" << command << ")\r\n";

    std::string msg = oss.str();
    send(client->getSocket(), msg.c_str(), msg.length(), 0);
}
