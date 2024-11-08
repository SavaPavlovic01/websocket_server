#include <iostream>
#include <openssl/sha.h>
#include "../inc/Server.hpp"

int main(){
    Logger log;
    Server server(log);
    server.run();
    
    return 0;
}