#pragma once
#include <string>
#include <sys/socket.h>

bool readUntilChar(const char delimiter, const int socket, std::string& dest){
    char buffer[2048];
    int charsRead = 0;
    while(true){
        if(charsRead >= 2048) return false;
        int bytesRead = recv(socket, buffer + charsRead, 1, 0);
        if(bytesRead == -1){
            return false;
        }
        charsRead += bytesRead;
        if(buffer[charsRead - 1] == delimiter){
            buffer[charsRead - 1] = '\0';
            dest = std::string(buffer);
            return true;
        }
    }
}

bool readUntilCharOrNewLine(const char delimiter, const int socket, std::string& dest){
    char buffer[2048];
    int charsRead = 0;
    while(true){
        if(charsRead >= 2048) return false;
        int bytesRead = recv(socket, buffer + charsRead, 1, 0);
        if(bytesRead == -1){
            return false;
        }
        charsRead += bytesRead;
        if(buffer[charsRead - 1] == '\n') return false;
        if(buffer[charsRead - 1] == delimiter){
            buffer[charsRead - 1] = '\0';
            
            dest = std::string(buffer);
            return true;
        }
    }
}