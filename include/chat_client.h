#pragma once
#include "asio.hpp"
#include "asio/connect.hpp"
#include "asio/impl/read.hpp"
#include "asio/io_context.hpp"
#include "asio/placeholders.hpp"
#include "glibmm/ustring.h"
#include "message.h"
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <format>
#include <system_error>

class ChatClient
{
    asio::io_context& m_ctx;
    asio::ip::tcp::socket m_socket;
public:
    ChatClient(asio::io_context& ctx, const char* name, uint16_t port_no) : 
        m_ctx(ctx),
        m_socket(ctx)
    {
        asio::ip::tcp::resolver resolver(ctx);
        char port_str[128] {};
        sprintf(port_str, "%u", (uint32_t)port_no);
        auto endpoints = resolver.resolve(name, port_str);
        asio::connect(m_socket, endpoints);
        GetServerMessage();
    }
    void ProcessServerMessage(const std::error_code& ec)
    {
        
    }
    void GetServerMessage()
    {
    }
    void SendMessage(const Glib::ustring& msg)
    {
        Message m;
        m.text = msg;
        m.time;
    }
    void Disconnect()
    {
    }
};
