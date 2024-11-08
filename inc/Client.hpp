#pragma once
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include "Logger.hpp"
#include <chrono>
#include <thread>
#include "RequestLine.hpp"
#include "RequestHeaders.hpp"
#include "Response.hpp"
#include <openssl/sha.h>
#include "base64.hpp"

class Client{
public:

    Client(const int socket, Logger& logger, int sleepTime): clientSocket(socket), logger(logger), sleepTime(sleepTime){}

    void run(){
        for(;;){
            RequestLine req(clientSocket, logger);
            RequestHeaders head(logger, clientSocket);
            
            if(req.method != "GET"){
                logger.logError("Only supports get method");
                break;
            }

            if(head.headers["Connection"] != " Upgrade"){
                logger.logError("Connection header wrong");
                break;
            }

            if(head.headers["Sec-WebSocket-Version"] != " 13"){
                logger.logError("Sec-WebSocket-Version header wrong");
                break;
            }

            if(!head.headers.contains("Sec-WebSocket-Key")){
                logger.logError("Sec-WebSocket-Key header missing");
                break;
            }

            if(head.headers["Upgrade"] != " websocket"){
                logger.logError("Upgrade header wrong");
            }
            logger.logInfo("Valid handshake initiation, sending response");
           

            Response resp;
            auto magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
            auto key = head.headers["Sec-WebSocket-Key"];
            key += magic;
            std::string_view toBeHashed = std::string_view(key).substr(1, key.length() - 1);
            unsigned char hash[20];
            SHA1((unsigned char *)toBeHashed.data(), toBeHashed.length(), hash);
            
            resp.sendWebSocketHandShakeResponse(clientSocket, base64::to_base64((char*) hash));
            
            logger.logInfo("Sent handshake response");
            break;
        }

    }

private:
    int sleepTime;
    const int clientSocket;
    Logger& logger;
};