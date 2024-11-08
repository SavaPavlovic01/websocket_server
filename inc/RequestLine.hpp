#pragma once
#include <string>
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
#include "util.hpp"


class RequestLine{
public:
    std::string method;
    std::string uri;
    std::string version;
    
    RequestLine(const int socket, Logger& logger):logger(logger){
        if(!readUntilChar(' ', socket, method) || !readUntilChar(' ', socket, uri) || !readUntilChar('\n', socket, version)){
            logger.logError("Malformed http request");
            return;
        }
        if(version.back() == '\r') version.pop_back(); 
        logger.logInfo(std::string("Parsed request line ") + method + uri + version);
    }

    RequestLine(Logger& logger): logger(logger){}
private:
    
    Logger& logger;
};