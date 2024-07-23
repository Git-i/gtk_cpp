#include "asio/io_context.hpp"
#include "chat_client.h"
#include <iostream>
#include <string>

int main (int argc, char *argv[]) {
    asio::io_context ctx;
    const char* name, *port;
    std::string name_str, port_str;
    if(argc < 3)
    {
        std::cout << "Enter Server Name/Address" << std::endl;;
        std::cin >> name_str;
        std::cout << "Enter Server Port" << std::endl;;
        std::cin >> port_str;
        name = name_str.c_str();
        port = port_str.c_str();
    }
    else {
        name = argv[1]; port = argv[2];
    }
    ChatClient client(ctx, name, port);
    std::string msg;
    while(true)
    {
        std::cout << "Enter Message" << std::endl;
        std::cin >> msg;
        if(msg == "-1") break;
        client.SendMessage(msg, 0);
        ctx.run();
    }
}
