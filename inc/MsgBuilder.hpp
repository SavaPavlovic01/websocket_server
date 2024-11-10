#pragma once
#include <inttypes.h>
#include <string>
#include <iostream>
#include "Logger.hpp"
#include <netinet/in.h>
#include <netinet/ip.h>
#include <byteswap.h>


enum class DataType{
    FRAGMENT,
    TEXT,
    BINARY,
    CLOSING = 0x8
};

class MsgBuilder{
public:

    void setDataType(const DataType type){
        opcode = (uint8_t)type;
        if(type != DataType::FRAGMENT){
            finBit = true;
        }
    }

    void setMaskKey(const uint32_t key){
        maskKey = key;
        mask = true;
    }

    void setCode(const uint16_t code){
        this->code = htons(code);
    }

    bool sendMsg(const std::string_view msg, const int socket){
        uint8_t firstByte = 0;
        if(finBit) firstByte |= 0x80;   
        if(rsv1) firstByte |= 0x40;     
        if(rsv2) firstByte |= 0x20;     
        if(rsv3) firstByte |= 0x10;     
        firstByte |= (opcode & 0x0F);   
        write(socket, &firstByte, 1);

        uint64_t payloadLen = msg.length();
        if(code != 0) payloadLen += 3;
        uint8_t secondByte = 0;
        if(mask) secondByte |= 0x80;
        if(payloadLen <= 125){
            secondByte |= ((uint8_t)payloadLen);
            write(socket, &secondByte, 1);
        }
        else if (payloadLen <= (0x10000 - 1)){
            secondByte |= 126;
            uint16_t len = htons(payloadLen);
            write(socket, &secondByte, 1);
            write(socket, &len, 2);
        }else{
            secondByte |= 127;
            uint64_t len = bswap_64(payloadLen);
            write(socket, &secondByte, 1);
            write(socket, &len, 4);
        }

        if(mask){
            write(socket, &maskKey, 4);
        }
        if(code != 0) {
            write(socket, &code, 2);
            write(socket, " ", 1);
        }
        write(socket, msg.data(), msg.length());
        return true;
    }

private:
    bool finBit = false;
    bool rsv1 = false;
    bool rsv2 = false;
    bool rsv3 = false;
    uint8_t opcode;
    bool mask = false;
    uint32_t maskKey;
    uint16_t code = 0;
};