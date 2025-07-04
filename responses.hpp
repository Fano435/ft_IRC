#ifndef RESPONSES_HPP
#define RESPONSES_HPP

#include <iostream>

#define RPL_WELCOME(client)                     "001 " + client + " :Welcome to the ft_irc Network"
#define RPL_TOPIC(client, channel, topic)       "332 " + client + " " + channel + " :" + topic
#define RPL_NAMREPLY(client, symbol, channel)   "353 " + client + " " + symbol + " " + channel + " :"
#define RPL_ENDOFNAMES(client, channel)         "366 " + client + " " + channel + " :End of /NAMES list"
#define RPL_YOUREOPER(client)                   "381 " + client + " :You are now an IRC operator"
#define RPL_UMODEIS(client, modes)              "221 " + client + " " + modes
#define RPL_CHANNELMODEIS(client, channel, modes) "324 " + client + " " + channel + " " + modes
#define ERR_UMODEUNKNOWNFLAG(client, mode)      "501 " + client + " :Unknown " + mode + " flag"
#define ERR_USERSDONTMATCH(client)              "502 " + client + " :Cant change mode for other users"
#define ERR_CHANOPIVSNEEDED(client, channel)    "482 " + client + " " + channel + " :You're not channel operator"

// #define ERR_NOSUCHNICK(source, nickname)                "401 " + source + " " + nickname + " :No such nick/channel"

#endif