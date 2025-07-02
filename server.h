#ifndef SERVER_H
#define SERVER_H

#include "Server.hpp"
#include "responses.hpp"
#include "errors.hpp"
#include "Channel.hpp"

std::vector<std::string> split(const std::string& input, char delimiter);
void sendError(Client* client, int code, const std::string& param = "");
void initErrors();
void sendNumeric(Client* client, int code, const std::string& command);

#endif