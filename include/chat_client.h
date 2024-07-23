#pragma once
#include "asio.hpp"
#include "asio/connect.hpp"
#include "asio/impl/read.hpp"
#include "asio/io_context.hpp"
#include "asio/placeholders.hpp"
#include "asio/write.hpp"
#include "glibmm/ustring.h"
#include "message.h"
#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <format>
#include <span>
#include <stdexcept>
#include <system_error>
#include <iostream>

class ChatClient
{
    asio::io_context& m_ctx;
    asio::ip::tcp::socket m_socket;
    uint32_t user_id;
public:
    ChatClient(asio::io_context& ctx, const char* name, const char* port_no) : 
        m_ctx(ctx),
        m_socket(ctx)
    {
        asio::ip::tcp::resolver resolver(ctx);
        auto endpoints = resolver.resolve(name, port_no);
        asio::connect(m_socket, endpoints);
        GetServerMessage();
    }
    void ProcessServerMessage(const std::error_code& ec, std::vector<std::byte> header)
    {
        auto [t, room_idx, msg_size] = Message::DecomposeHeader(header);
        if(t == MessageType::Communication)
        {
            std::vector<std::byte> msg_buf((size_t)msg_size+8);
            asio::read(m_socket, asio::buffer(msg_buf));
            Message m = Message::DecomposeChatBody(msg_buf);
            std::cout << "User " << m.user << " said: " << m.text << std::endl;
        }
        GetServerMessage();
    }
    void GetServerMessage()
    {
        std::vector<std::byte> header(9);
        asio::async_read(m_socket, asio::buffer(header.data(), header.size()), std::bind(&ChatClient::ProcessServerMessage, this, asio::placeholders::error, header));
    }
    void SendMessage(const Glib::ustring& msg, uint32_t room_idx)
    {
        auto message = Message::MakeForForwarding(MessageType::Communication, room_idx, user_id, msg);
        asio::write(m_socket, asio::buffer(message));
    }
    void Disconnect()
    {
    }
};
