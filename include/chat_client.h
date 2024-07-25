#pragma once
#include "asio.hpp"
#include "asio/connect.hpp"
#include "asio/impl/read.hpp"
#include "asio/io_context.hpp"
#include "asio/placeholders.hpp"
#include "asio/write.hpp"
#include "glibmm/ustring.h"
#include "message.h"
#include "pangomm/color.h"
#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <format>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <system_error>
#include <iostream>
#include <unordered_map>
struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    static Color Random()
    {
        return Color{
            (uint8_t)(rand() % 256),
            (uint8_t)(rand() % 256),
            (uint8_t)(rand() % 256),
            (uint8_t)(rand() % 256)
        };
    }
};
class ChatClient
{
    asio::io_context& m_ctx;
    asio::ip::tcp::socket m_socket;
    uint32_t user_id;
    Glib::ustring name;
    std::array<std::byte, Message::header_length> header;
    std::unordered_map<uint32_t, Glib::ustring> usernames;
    std::unordered_map<uint32_t, Color> colors;
public:
    std::function<void(const Chat& /*message*/)> on_message;
    ChatClient(asio::io_context& ctx, const char* name, const char* port_no, const Glib::ustring& user_name) : 
        m_ctx(ctx),
        m_socket(ctx)
    {
        asio::ip::tcp::resolver resolver(ctx);
        auto endpoints = resolver.resolve(name, port_no);
        asio::connect(m_socket, endpoints);
        this->name = user_name;
        colors[std::numeric_limits<uint32_t>::max()] = Color{200, 40, 20, 255};
        GetServerMessage();
    }
    uint32_t GetId()
    {
        return user_id;
    }
    Color GetUserColor(uint32_t id)
    {
        if(colors.contains(id))
        {
            return colors[id];
        }
        else
        {
            colors[id] = Color::Random();
            return GetUserColor(id);
        }
    }
    void ProcessServerMessage(const std::error_code& ec)
    {
        auto [t, room_idx, msg_size] = Message::DecomposeHeader(header);
        std::cout << "recieved message of size: " << msg_size << " byte(s)" << std::endl;
        switch (t) {
            case MessageType::Communication:
            {
                std::vector<std::byte> msg_buf((size_t)msg_size+8);
                asio::read(m_socket, asio::buffer(msg_buf));
                Chat ch = Message::DecomposeChatBody(msg_buf);
                if(on_message)
                {
                    on_message(ch);
                }
                
                break;
            }
            case MessageType::UserId:
            {
                user_id = room_idx;
                SendUserDetail();
                break;
            }
            case MessageType::UserDetails:
            {
                std::vector<std::byte> msg_buf((size_t)msg_size);
                asio::read(m_socket, asio::buffer(msg_buf));
                if(room_idx != user_id)
                {
                    usernames[room_idx] = Message::DecomposeUserDetailBody(msg_buf);
                }
                break;
            }
        }
        GetServerMessage();
    }
    void SendUserDetail()
    {
        auto msg = Message::MakeUserDetailForForwarding(name, user_id);
        asio::write(m_socket, asio::buffer(msg));
    }
    Glib::ustring GetUserName(uint32_t id)
    {
        if(id == user_id) return "You";
        if(usernames.contains(id))
        {
            return usernames[id] + "-" + std::to_string(id);
        }
        else 
        {
            asio::write(m_socket,
                asio::buffer(Message::MakeUserDetailForForwarding("", id)));
            while(true)
            {
                std::error_code e;
                asio::read(m_socket, asio::buffer(header), e);
                if(e) break;
                if(std::bit_cast<MessageType>(header[0]) != MessageType::UserDetails)
                {
                    ProcessServerMessage(e);
                }
                else
                {
                    auto[_, id, size] = Message::DecomposeHeader(header);
                    std::vector<std::byte> msg_buf(size);
                    asio::read(m_socket, asio::buffer(msg_buf));
                    usernames[id] = Message::DecomposeUserDetailBody(msg_buf);
                    break;
                }
            }
            return GetUserName(id);
        }
    }
    void GetServerMessage()
    {
        asio::async_read(m_socket, asio::buffer(header), std::bind(&ChatClient::ProcessServerMessage, this, asio::placeholders::error));
    }
    void SendMessage(const Glib::ustring& msg, uint32_t room_idx)
    {
        Chat ch;
        ch.text = msg;
        const auto message = Message::MakeChatForForwarding(room_idx, ch);
        asio::write(m_socket, asio::buffer(message));
    }
    void Disconnect()
    {
    }
};
