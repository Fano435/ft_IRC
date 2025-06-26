#ifndef SERVER_H
#define SERVER_H

#include "Server.hpp"
#include "errors.hpp"

void sendError(Client* client, int code, const std::string& param = "");
void sendRWelcome(Client* client);
void initErrors();

#endif