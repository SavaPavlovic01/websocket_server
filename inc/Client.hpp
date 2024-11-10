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
#include "MsgBuilder.hpp"
#include <poll.h>
#include "MsgReader.hpp"

class Client{
public:

    Client(const int socket, Logger& logger, int sleepTime): clientSocket(socket), logger(logger), sleepTime(sleepTime){}

    void run(){
        RequestLine req(clientSocket, logger);
        RequestHeaders head(logger, clientSocket);

        if(!validateRequest(req, head)) return;
        performHandshake(head);

        for(;;){
            MsgReader reader(-1);
            try{
                auto res = reader.getMsg(clientSocket);
                if(res.has_value() && res.value().opcode == DataType::CLOSING){
                    MsgBuilder b;
                    sendCloseMsg(1000, "");
                    close(clientSocket);
                    break;
                } else if(res.has_value()){
                    MsgBuilder msg;
                    msg.setDataType(DataType::TEXT);
                    msg.sendMsg("Hello", clientSocket);
                } else {
                    // if we set timeout != -1, we can send heartbeat msg here
                }
                logger.logInfo(res.value().msg);
            }
            catch(const std::exception& e){
                sendCloseMsg(1002, e.what());
                logger.logError(e.what());
                close(clientSocket);
                break;
            }
            
        }

    }

private:

    void sendCloseMsg(uint16_t code, const std::string& msg){
        MsgBuilder builder;
        builder.setDataType(DataType::CLOSING);
        builder.setCode(code);
        builder.sendMsg(msg, clientSocket);
    }

    bool performHandshake(RequestHeaders& head){
        Response resp;
        auto magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        auto key = head.headers["Sec-WebSocket-Key"];
        key += magic;
        std::string_view toBeHashed = std::string_view(key).substr(1, key.length() - 1);
        unsigned char hash[20];
        SHA1((unsigned char *)toBeHashed.data(), toBeHashed.length(), hash);
        
        resp.sendWebSocketHandShakeResponse(clientSocket, base64::to_base64((char*) hash));
        
        logger.logInfo("Sent handshake response");
        return true;
    }

    bool validateRequest(const RequestLine& req, RequestHeaders& head){
        if(req.method != "GET"){
            logger.logError("Only supports get method");
            return false;
        }

        if(head.headers["Connection"] != " Upgrade"){
            logger.logError("Connection header wrong");
            return false;
        }

        if(head.headers["Sec-WebSocket-Version"] != " 13"){
            logger.logError("Sec-WebSocket-Version header wrong");
            return false;
        }

        if(!head.headers.contains("Sec-WebSocket-Key")){
            logger.logError("Sec-WebSocket-Key header missing");
            return false;
        }

        if(head.headers["Upgrade"] != " websocket"){
            logger.logError("Upgrade header wrong");
            return false;
        }
        logger.logInfo("Valid handshake initiation, sending response");
        return true;
    }

    int sleepTime;
    const int clientSocket;
    Logger& logger;
};