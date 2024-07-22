#include <gtkmm.h>
#include <memory>
#include <system_error>
#include "asio/io_context.hpp"
#include "asio/placeholders.hpp"
#include "main_window.h"
#include "chat_server.h"
#include "asio.hpp"
#include <iostream>
using asio::ip::tcp;

int main (int argc, char *argv[]) {
    try
  {
    asio::io_context io_context;

    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 2010));

    for (;;)
    {
      tcp::socket socket(io_context);
      acceptor.accept(socket);

      std::string message = "rwar";
      
      std::error_code ignored_error;
      asio::write(socket, asio::buffer(message), ignored_error);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
    auto app = Gtk::Application::create("org.git_i.lchat");
    app->make_window_and_run<MainWindow>(argc, argv);
}
