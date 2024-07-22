#include "asio/io_context.hpp"
#include "chat_server.h"
#include <exception>
int main()
{
    try 
    {
        asio::io_context ctx;
        ChatServer server(ctx, 2010);
        ctx.run();
    } 
    catch (const std::exception& e) 
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
}