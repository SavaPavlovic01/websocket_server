#pragma once
#include "Logger.hpp"
#include <unordered_map>
#include "util.hpp"

class RequestHeaders{
public:
    std::unordered_map<std::string, std::string> headers;
    RequestHeaders(Logger& logger, const int socket):logger(logger){
        while(true){
            std::string curHeader;
            if(!readUntilCharOrNewLine(':', socket, curHeader)){
                break;
            }
            std::string curValue;
            readUntilChar('\n', socket, curValue);
            if(curValue.back() == '\r' || curValue.back() == '\n') curValue.pop_back();
            headers[std::move(curHeader)] = std::move(curValue);
        }
    }

    void logData(){
        for(const auto& i:headers){
            std::cout<<i.first <<":"<<i.second<<std::endl;
        }
    }
private:
    Logger& logger;
    
};