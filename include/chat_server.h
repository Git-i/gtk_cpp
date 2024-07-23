#pragma once
#include "asio.hpp"
#include "asio/impl/read.hpp"
#include "asio/placeholders.hpp"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <system_error>
#include <vector>
#include "message.h"
class ChatConnection
{
private:
    asio::ip::tcp::socket m_socket;
    std::vector<std::byte> m_buffer;
    ChatConnection(asio::io_context& ctx): m_socket(ctx) {m_buffer.resize(12);}
public:
    static std::shared_ptr<ChatConnection> Create(asio::io_context& ctx)
    {
        return std::shared_ptr<ChatConnection>(new ChatConnection(ctx));
    }
    asio::ip::tcp::socket& socket()
    {
        return m_socket;
    }
    std::vector<std::byte>& buffer()
    {
        return m_buffer;
    }
};
class ChatServer
{
    asio::io_context& m_ctx;
    asio::ip::tcp::acceptor m_acceptor;
    std::vector<std::shared_ptr<ChatConnection>> connections;
    static constexpr size_t header_length = 1/*purpose*/+ 4/*room idx*/+ 4/*message lenght*/ ;
public:
    ChatServer(asio::io_context& ctx, uint16_t port_no) : 
        m_ctx(ctx),
        m_acceptor(m_ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_no))
    {
        listen();
    }
    void listen()
    {
        auto connection = ChatConnection::Create(m_ctx);
        m_acceptor.async_accept(connection->socket(), std::bind(&ChatServer::on_accept, this, asio::placeholders::error, connection));
    }
    void on_message(const std::error_code& error, std::shared_ptr<ChatConnection> connection)
    {
        std::cout << "msg recieved" << std::endl;
        MessageType t = std::bit_cast<MessageType>(connection->buffer()[0]);
        uint32_t room_idx = *reinterpret_cast<uint32_t*>((char*)connection->buffer().data() + 1);
        uint32_t msg_size = *reinterpret_cast<uint32_t*>((char*)connection->buffer().data() + 5);
        if(std::endian::native != std::endian::little)
        {
            std::reverse((char*)&room_idx, (char*)&room_idx + 4);
            std::reverse((char*)&msg_size, (char*)&msg_size + 4);
        }
        if(t == MessageType::Communication)
        {
            std::vector<std::byte> msg_buf((size_t)msg_size+8);
            asio::read(connection->socket(), asio::buffer(msg_buf.data(), msg_buf.size()));
            Message m = Message::FromBytes(std::span<std::byte>(msg_buf.begin(), msg_buf.end()), msg_size);
            std::cout << "Recieved Message: " << m.text << std::endl;
        }
        else if (t == MessageType::LeaveRequest)
        {
            connections.erase(std::remove(connections.begin(), connections.end(), connection));
        }
    }
    void on_accept(const std::error_code& error, std::shared_ptr<ChatConnection> connection)
    {
        if(!error)
        {
            asio::async_read(connection->socket(), asio::buffer(connection->buffer().data(), header_length), std::bind(&ChatServer::on_message, this, 
            asio::placeholders::error,
            connection));
            connections.push_back(connection);
        }
        listen();
    }
};