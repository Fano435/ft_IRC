#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "server.h"
#include <set>
#include <list>

class Server;

class Channel
{
    public:
        Channel(Server &server, Client* admin, const std::string name);
        void addClient(Client* client, std::string name );
        void removeClient(Client* client, const std::string &msg);
        void broadcast(Client* client, const std::string &message, int exclude_fd = -1 );
        std::string listUsers();
        void setTopic(const std::string topic);
        std::string getTopic() const;
        std::string getMode() const;
        // void change_priv(Client *client, std::string nick);
        void setMode(Client *client, const std::vector<std::string>& args);
        ~Channel();

    private:
        Server &_server;
        std::string _name;
        std::set<Client *> _admins;
        std::map<std::string, Client *> _clients;

        bool _i;        // invitation canal
        bool _t;        // topic protection
        std::string _k; // key (password)
        size_t _l;      // limit of users
        std::string _topic;
        std::set<char> _modes;

};

#endif