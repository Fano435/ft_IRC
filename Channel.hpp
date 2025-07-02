#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "server.h"

class Channel
{
    public:
        Channel(Client* admin, const std::string name);
        void addClient(Client* client, std::string name );
        void removeClient(Client* client, const std::string &msg);
        void broadcast(Client* client, const std::string &message, int exclude_fd = -1 );
        std::string listUsers();
        void setTopic(const std::string topic);
        std::string getTopic() const;
        ~Channel();

    private:
        Client* _admin;
        std::string _name;
        std::map<int, Client *> _clients;

        size_t _l; //limit of users
        std::string _topic;
        std::string _mode;

};

#endif