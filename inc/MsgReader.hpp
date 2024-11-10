#pragma once
#include <poll.h>
#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <optional>
#include "MsgBuilder.hpp"

typedef struct WSMsg{
    DataType opcode;
    std::string msg;
} WSMsg;

class MsgReader{
public:
    MsgReader(const int timeout):timeout(timeout){}

    // blocks if timeout = -1
    std::optional<WSMsg> getMsg(const int socket){
        if(hasDataToRead(socket)){
            uint8_t firstByte;
            recv(socket, &firstByte, 1, 0);
            if(firstByte & 0x40) throw std::invalid_argument("rsv1 has to be clear");
            if(firstByte & 0x20) throw std::invalid_argument("rsv2 has to be clear");
            if(firstByte & 0x10) throw std::invalid_argument("rsv3 has to be clear");
            uint8_t opcode = firstByte & 0x0f;
            DataType dt;
            switch (opcode){
                case 0x00: dt = DataType::FRAGMENT; break;
                case 0x01: dt = DataType::TEXT; break;
                case 0x02: dt = DataType::BINARY; break;
                case 0x08: dt = DataType::CLOSING; break;
                default: throw std::invalid_argument("invalid opcode"); break;
            }

            uint8_t secondByte;
            recv(socket, &secondByte, 1, 0);
            if(!(secondByte & 0x80)) throw std::invalid_argument("Mask has to be set");
            uint8_t len = secondByte & 0x7f;
            uint64_t real_len;
            if(len == 126){
                recv(socket, &real_len, 2, 0);
                real_len = ntohl(real_len);
            } else if(len == 127) {
                recv(socket, &real_len, 8, 0);
                // switch from network to host byte order here
            }
            else real_len = len;

            uint32_t key;
            recv(socket, &key, 4, 0);
            key = ntohl(key);
            char msg[real_len];
            recv(socket, msg, real_len, 0);
            for(int i = 0; i < real_len; i++){
                uint8_t curKey = key >> (((3 - (i % 4) ) * 8) & 0x00ff);
                msg[i] = (msg[i] ^ curKey);
            }

            return WSMsg{dt, std::string(msg)};

        }else return std::nullopt;
    }
private:

    bool hasDataToRead(const int socket_fd) {
        struct pollfd pfd;
        pfd.fd = socket_fd;
        pfd.events = POLLIN; // Check for readability

        // Poll the socket with the specified timeout
        int result = poll(&pfd, 1, timeout);

        if (result < 0) {
            std::cerr << "poll() error: " << strerror(errno) << std::endl;
            return false;
        }
        return result > 0 && (pfd.revents & POLLIN);  // True if socket is ready to read
    }

    const int timeout = -1;

};
