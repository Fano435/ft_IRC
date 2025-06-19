#include "server.h"

bool isValidNickname(const std::string& nick) {
    if (nick.empty())
        return false;

    char first = nick[0];

    if (first == ':' || first == '#' || std::isdigit(first))
        return false;

    for (size_t i = 0; i < nick.length(); ++i) {
        char c = nick[i];

        if (std::isalnum(c))
            continue;

        if (c == '[' || c == ']' ||
            c == '{' || c == '}' ||
            c == '\\' || c == '|')
            continue;

        if (std::isspace(c))
            return false;

        return false;
    }

    return true;
}

void Server::initReplies() {
    _errors[ERR_NONICKNAMEGIVEN] = "No nickname given";
    _errors[ERR_ERRONEUSNICKNAME] = "Erroneous nickname";
    _errors[ERR_NICKNAMEINUSE] = "Nickname is already in use";
    _errors[ERR_UNKNOWNCOMMAND] = "Unknown command";
    _errors[ERR_NEEDMOREPARAMS] = "Not enough parameters";
    _errors[ERR_PASSWDMISMATCH] = "Password incorrect";
    _errors[ERR_ALREADYREGISTRED] = "You may not reregister";
}
