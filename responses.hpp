#ifndef RESPONSES_HPP
#define RESPONSES_HPP

#include <iostream>

#define RPL_WELCOME(client)                     "001 " + client + " :Welcome to the ft_irc Network"
#define RPL_TOPIC(client, channel, topic)       "332 " + client + " " + channel + " :" + topic
#define RPL_NAMREPLY(client, symbol, channel)   "353 " + client + " " + symbol + " " + channel + " :"
#define RPL_ENDOFNAMES(client, channel)         "366 " + client + " " + channel + " :End of /NAMES list"

// #define ERR_NOSUCHNICK(source, nickname)                "401 " + source + " " + nickname + " :No such nick/channel"

#endif