#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "server.h"
#include <ctime>
#include <iostream>
#include <set>
#include <list>

class Server;

class Channel
{
    public:
        Channel(Server &server, Client* admin, const std::string name);
        void add(Client* client);
        void remove(Client* client, const std::string &msg);
        void broadcast(Client* client, const std::string &message, int exclude_fd = -1 );
        std::string listUsers();
        void setTopic(Client *client, const std::string topic);
        std::string getTopic() const;
        std::string getMode() const;
        bool is_full();
        bool check_key(const std::string &key);
        bool has_client(Client *client);
        Client *getClient(const std::string &name);
        bool is_admin(Client *client);
        void invite(Client *client, const std::string &target);
        void kick(Client *client, const std::vector<std::string>& args);
        void setMode(Client *client, const std::vector<std::string>& args);
        ~Channel();
        std::time_t get_topic_time() const;

    private:
        Server &_server;
        std::string _name;
        std::set<Client *> _invited;
        std::set<Client *> _admins;
        std::map<int, Client *> _clients;

        bool _i;        // invitation canal
        bool _t;        // topic protection
        std::string _k; // key (password)
        size_t _l;      // limit of users

        std::string _topic;
        std::string _mode;
        std::time_t _topic_time;

        std::set<char> _modes;
};

#endif