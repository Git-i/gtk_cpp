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

class ChatClient
{
    asio::io_context& m_ctx;
    asio::ip::tcp::socket m_socket;
    uint32_t user_id;
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
    void SendMessage(const Glib::ustring& msg, uint32_t room_idx)
    {
        if(msg.bytes() > (size_t)UINT32_MAX)
        {
            throw std::runtime_error("Message too long");
        }
        Message m;
        m.text = msg;
        m.user = user_id;
        std::vector<std::byte> message;
        message.push_back(std::bit_cast<std::byte>(MessageType::Communication));
        auto room_span = std::as_writable_bytes(std::span<uint32_t>(&room_idx,1));
        if(std::endian::native == std::endian::little)
        {
            std::reverse(room_span.begin(), room_span.end());
        }
        message.insert(message.end(), room_span.begin(), room_span.end());
        uint32_t msg_len = msg.bytes();
        auto len_span = std::as_writable_bytes(std::span<uint32_t>(&msg_len, 1));
        if(std::endian::native == std::endian::little)
        {
            std::reverse(len_span.begin(), len_span.end());
        }
        message.insert(message.end(), len_span.begin(), len_span.end());
        auto m_bytes = m.IntoBytes();
        message.insert(message.end(), m_bytes.begin(), m_bytes.end());
        asio::write(m_socket, asio::buffer(message));
    }
    void Disconnect()
    {
    }
};
