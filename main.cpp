#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include <cstring>

#define MAX_EVENTS 10

int main()
{    
    int server_sock;
    struct sockaddr_in server_addr;
    // char buffer[1024] ?;
    int port = 6667; // Default IRC port


    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        return 1;
    }

    // memset(&server_addr, 0, sizeof(server_addr)) ?;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_sock);
        return 1;
    }

    if (listen(server_sock, MAX_EVENTS))
    {
        perror("listen");
        close(server_sock);
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct epoll_event ev, events[MAX_EVENTS];
    int client_sock, nfds, epoll_fd;

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) 
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = server_sock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_sock, &ev) == -1 )
    {
        perror("epoll_ctl: server_sock");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) 
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == server_sock)
            {
                client_sock = accept4(server_sock, (struct sockaddr *)&client_addr, &client_addr_len, SOCK_NONBLOCK);
                if (client_sock == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                std::cout << "Nouvelle connexion acceptee" << std::endl;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_sock;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sock, &ev) == -1 )
                {
                    perror("epoll_ctl: client_sock");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    /*Ce n'est pas une nouvelle connexion donc il s'agit de gerer un socket existant (lire/ecrire)*/
                }
            }
        }
    }
    return 0;
}