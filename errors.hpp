#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <map>
#include <iostream>

extern std::map<int, std::string> errors;

#define ERR_NOSUCHNICK        401
#define ERR_NOSUCHSERVER      402
#define ERR_NOSUCHCHANNEL     403
#define ERR_NOORIGIN          409
#define ERR_NORECIPIENT       411
#define ERR_NOTEXTTOSEND      412
#define ERR_UNKNOWNCOMMAND    421
#define ERR_NONICKNAMEGIVEN   431
#define ERR_ERRONEUSNICKNAME  432
#define ERR_NICKNAMEINUSE     433
#define ERR_USERNOTINCHANNEL  441
#define ERR_NOTONCHANNEL      442
#define ERR_NEEDMOREPARAMS    461
#define ERR_ALREADYREGISTRED  462
#define ERR_PASSWDMISMATCH    464
#define ERR_CHANNELISFULL     471


#endif