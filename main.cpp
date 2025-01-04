#include "asio/io_context.hpp"
#include "chat_client.h"
#include "gtkmm/application.h"
#include <iostream>
#include <string>
#include <thread>
#include "gtkmm/applicationwindow.h"
#include "main_window.h"
#include "sigc++/functors/mem_fun.h"
int main2 (int argc, char *argv[]) {
    asio::io_context ctx;
    const char* name, *port, *user_name;
    std::string name_str, port_str, usr_name;

    if(argc < 4)
    {
        std::cout << "Enter Server Name/Address" << std::endl;;
        std::cin >> name_str;
        std::cout << "Enter Server Port" << std::endl;;
        std::cin >> port_str;
        std::cout << "Enter User Name" << std::endl;;
        std::cin >> usr_name;
        
        name = name_str.c_str();
        port = port_str.c_str();
        user_name = usr_name.c_str();
    }
    else {
        name = argv[1]; port = argv[2]; user_name = argv[3];
    }
    ChatClient client(ctx, name, port, user_name);
    
    std::thread t([&]{ctx.run();});
    std::string msg;
    while(true)
    {
        std::cin >> msg;
        if(msg == "-1") break;
        client.SendMessage(msg, 0);
    }
}
int main(int argc, char** argv)
{
    srand(time(NULL));
    asio::io_context ctx;
    const char* name, *port, *user_name;
    std::string name_str, port_str, usr_name;

    if(argc < 4)
    {
        std::cout << "Enter Server Name/Address" << std::endl;;
        std::cin >> name_str;
        std::cout << "Enter Server Port" << std::endl;;
        std::cin >> port_str;
        std::cout << "Enter User Name" << std::endl;;
        std::cin >> usr_name;
        
        name = name_str.c_str();
        port = port_str.c_str();
        user_name = usr_name.c_str();
    }
    else {
        name = argv[1]; port = argv[2]; user_name = argv[3];
    }
    Glib::ustring appid = "org.git_i.lchat" + std::to_string(rand());
    auto app  = Gtk::Application::create(appid);
    auto status = app->make_window_and_run<MainWindow>(argc, argv, ctx, name, port, user_name);
    std::cout << status;
    return status;
}
    