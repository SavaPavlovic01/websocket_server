#pragma once
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include "Logger.hpp"
#include "Client.hpp"
#include <thread>

class Server{
    public:
    Server(Logger& logger):tcpSocket(socket(AF_INET, SOCK_STREAM, 0)), logger_(logger){
        struct sockaddr_in my_addr;
        memset(&my_addr, 0, sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = 80;
        struct in_addr socket_addr;
        socket_addr.s_addr = htonl(INADDR_LOOPBACK);
        my_addr.sin_addr = socket_addr;
        int opt = 1;
        setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if(bind(tcpSocket, (struct sockaddr *)&my_addr, sizeof(my_addr)) != 0){
            logger_.logError("Server failed on bind");
            exit(-1);
        }
        logger_.logInfo("Bind succesfull");
        
        if(listen(tcpSocket, 20) != 0){
            logger_.logError("Server failed on listen");
            exit(-1);
        }
        logger_.logInfo(std::string("Server listening on port ") + std::to_string(getPortNumber()));
        
    }

    void run(){
        for(;;){
            struct sockaddr_in client;

            unsigned len = sizeof(client);
            const int client_socket = accept(tcpSocket, (struct sockaddr *)&client, &len);
            
            if(client_socket == -1){
                logger_.logError("Accept failed");
                continue;
            }
            int opt = 1;
            setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            logger_.logInfo("Client connected");
            clients.push_back(Client(client_socket, logger_, client_socket));
            std::thread clientThread(&Client::run, clients.back());
            clientThread.detach();
        }
    }

    ~Server(){
        close(tcpSocket);
    }

    private:

        int getPortNumber(){
            struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            if (getsockname(tcpSocket, (struct sockaddr *)&sin, &len) == -1){
                logger_.logWarning("Failed getting port number");
                return -1;
            }
            else
                return ntohs(sin.sin_port);
        }

        const int tcpSocket;
        std::vector<Client> clients;
        Logger& logger_;
};