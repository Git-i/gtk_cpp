#pragma once
#include "asio.hpp"
#include "asio/connect.hpp"
#include "asio/impl/read.hpp"
#include "asio/io_context.hpp"
#include "glibmm/ustring.h"
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <format>

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
        UpdateMessageList();
    }
    void UpdateMessageList()
    {
        asio::read(m_socket, asio::buffer((char*)nullptr, 0));
    }
    void SendMessage(const Glib::ustring& msg)
    {
    }
    void Disconnect()
    {
    }
};
