#pragma once
#include <string>
#include <unordered_map>

#include "Logger.hpp"

class Response{
public:
    Response(){}

    void setVersion(std::string_view version){
        this->version = version;
    }
    void addHeader(std::string_view header, std::string_view value){
        headers[std::string(header)] = value;
    }
    void setCode(std::string_view code){
        this->code = code;
    }
    void setMsg(std::string_view msg){
        this->msg = msg;
    }

    bool sendResponse(const int socket){
        write(socket, "HTTP/", 5);
        write(socket, version.c_str(), version.length());
        write(socket, " ", 1);
        write(socket, code.c_str(), code.length());
        write(socket, " ", 1);
        write(socket, msg.c_str(), msg.length());
        write(socket, "\r\n", 2);
        for(const auto& pair: headers){
            write(socket, pair.first.c_str(), pair.first.length());
            write(socket, ": ", 2);
            write(socket, pair.second.c_str(), pair.second.length());
            write(socket, "\r\n", 2);
        }
        write(socket, "\r\n", 2);
        return true;
    }

    // TODO: check if write returns -1
    bool sendWebSocketHandShakeResponse(const int socket, std::string_view key){
        bool error = false;
        error = (write(socket, "HTTP/", 5) == -1) || error;
        write(socket, "1.1", 3);
        write(socket, " ", 1);
        write(socket, "101", 3);
        write(socket, " ", 1);
        write(socket, "Switching Protocols", 19);
        write(socket, "\r\n", 2);
        write(socket, "Upgrade: ", 9);
        write(socket, "websocket\r\n", 11);
        write(socket, "Connection: ", 12);
        write(socket, "Upgrade\r\n", 9);
        write(socket, "Sec-WebSocket-Accept: ", 22);
        write(socket, key.data(), key.length());
        write(socket, "\r\n", 2);
        write(socket, "\r\n", 2);
        return true;
    }
private:
    std::string version = "1.1";
    std::string code;
    std::string msg;
    std::unordered_map<std::string, std::string> headers;
};