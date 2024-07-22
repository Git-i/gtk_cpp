#pragma once
#include "asio.hpp"
#include "asio/impl/read.hpp"
#include "asio/placeholders.hpp"
#include "glibmm/datetime.h"
#include "glibmm/ustring.h"
#include <algorithm>
#include <bit>
#include <codecvt>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <span>
#include <system_error>
#include <vector>
enum class MessageType : uint8_t
{
    Communication = 0,
    LeaveRequest = 1
};
struct DateTimeNetwork
{
    //for time we need an accuracy in minutes and theres only 1440 mins
    uint32_t time_of_day:11;
    uint32_t year:5;
    uint32_t month:4;
    uint32_t day: 5;
};
struct DateTime
{
    uint32_t year;
    uint32_t day;
    uint32_t month;
    uint32_t min_of_day;
    DateTimeNetwork ToNetwork()
    {
        return DateTimeNetwork{
            min_of_day,
            year,
            month,
            day
        };
    }
    DateTime() = default;
    DateTime(DateTimeNetwork net)
    {
        min_of_day = net.time_of_day;
        year = net.year;
        month = net.month;
        day = net.day;
    }
};
struct Message
{
    Glib::ustring text;
    DateTime time;
    std::vector<uint64_t> attachments;
    std::vector<std::byte> IntoBytes()
    {
        std::vector<std::byte> returnVal;
        auto net_time = time.ToNetwork();
        std::span<std::byte, sizeof(DateTimeNetwork)> net_time_bytes(reinterpret_cast<std::byte*>(&net_time), 4);
        if(std::endian::native != std::endian::little)
        {
            std::reverse(net_time_bytes.begin(), net_time_bytes.end());
        }
        returnVal.insert(returnVal.end(), net_time_bytes.begin(), net_time_bytes.end());
        auto old_size = returnVal.size();
        auto text_size = text.bytes();
        returnVal.resize(old_size + text_size);
        text.copy(reinterpret_cast<char*>(returnVal.data()) + old_size, text_size);
        //TODO attachments;
        return returnVal;
    }
    static Message FromBytes(std::span<std::byte> bytes)
    {
        Message msg;
        std::span<std::byte, sizeof(DateTimeNetwork)> net_time_bytes(bytes.data(), 4);
        if(std::endian::native != std::endian::little)
        {
            std::reverse(net_time_bytes.begin(), net_time_bytes.end());
        }
        DateTimeNetwork net_time = *reinterpret_cast<DateTimeNetwork*>(net_time_bytes.data());
        msg.time = DateTime(net_time);
        msg.text = Glib::ustring(reinterpret_cast<char*>(bytes.data()) + 4, bytes.size() - 4);
        return msg;
    }
};
class ChatConnection
{
private:
    asio::ip::tcp::socket m_socket;
    std::vector<std::byte> m_buffer;
    ChatConnection(asio::io_context& ctx): m_socket(ctx) {}
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
    const size_t header_length = 4/*time*/ + 4/*message lenght*/;
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
        if(t == MessageType::Communication)
        {
            auto span = std::span<std::byte>(connection->buffer().begin()+1, connection->buffer().end());
            Message m = Message::FromBytes(span);
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