#pragma once
#include "glibmm/ustring.h"
#include <ctime>
#include <span>
#include <cstdint>
#include <vector>
#include <algorithm>
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
    DateTimeNetwork ToNetwork() const
    {
        return DateTimeNetwork{
            min_of_day,
            year,
            month,
            day
        };
    }
    static DateTime now()
    {
        DateTime dt;
        std::time_t t = std::time(0);
        std::tm* now = std::localtime(&t);
        now->tm_hour;   
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
struct Chat
{
    Glib::ustring text;
    DateTime time;
    uint32_t user;
    std::vector<uint64_t> attachments;
};
class Message
{
public:
    static constexpr size_t header_length = 1/*purpose*/+ 4/*room idx*/+ 4/*message lenght*/ ;
    static std::vector<std::byte> ComposeChatBody(const Chat& ch)
    {
        std::vector<std::byte> returnVal;
        uint32_t user_le = ch.user;
        auto net_time = ch.time.ToNetwork();
        std::span<std::byte, sizeof(DateTimeNetwork)> net_time_bytes(reinterpret_cast<std::byte*>(&net_time), 4);
        if(std::endian::native != std::endian::little)
        {
            std::reverse(net_time_bytes.begin(), net_time_bytes.end());
            std::reverse((char*)&user_le, ((char*)&user_le)+4);
        }
        returnVal.insert(returnVal.end(), net_time_bytes.begin(), net_time_bytes.end());
        auto user_le_span = std::as_bytes(std::span<uint32_t>(&user_le, 1));
        returnVal.insert(returnVal.end(), user_le_span.begin(), user_le_span.end());
        auto old_size = returnVal.size();
        auto text_size = ch.text.bytes();
        returnVal.resize(old_size + text_size);
        ch.text.copy(reinterpret_cast<char*>(returnVal.data()) + old_size, text_size);
        //TODO attachments;
        return returnVal;
    }
    static Chat DecomposeChatBody(std::span<std::byte> bytes)
    {
        Chat msg;
        std::span<std::byte, sizeof(DateTimeNetwork)> net_time_bytes(bytes.data(), 4);
        uint32_t user = *reinterpret_cast<uint32_t*>((char*)bytes.data() + sizeof(DateTimeNetwork));
        if(std::endian::native != std::endian::little)
        {
            std::reverse(net_time_bytes.begin(), net_time_bytes.end());
            std::reverse((char*)&user, ((char*)&user)+4);
        }
        DateTimeNetwork net_time = *reinterpret_cast<DateTimeNetwork*>(net_time_bytes.data());
        msg.time = DateTime(net_time);
        msg.user = user;
        msg.text = Glib::ustring(reinterpret_cast<char*>(bytes.data()) + 8, bytes.size() - 8);
        return msg;
    }
    static std::vector<std::byte> MakeChatForForwarding(uint32_t ri, uint32_t user,const Chat& ch)
    {
        std::vector<std::byte> message;
        auto header = ComposeChatHeader(ri, ch);
        message.insert(message.begin(), header.begin(), header.end());
        auto body = ComposeChatBody(ch);
        message.insert(message.end(), body.begin(), body.end());
        return message;
    }
    static std::tuple<MessageType, uint32_t, uint32_t> DecomposeHeader(std::span<std::byte> header)
    {
        using return_t = std::tuple<MessageType, uint32_t, uint32_t>;
        MessageType t = std::bit_cast<MessageType>(header[0]);
        uint32_t room_idx = *reinterpret_cast<uint32_t*>((char*)header.data() + 1);
        uint32_t msg_size = *reinterpret_cast<uint32_t*>((char*)header.data() + 5);
        if(std::endian::native != std::endian::little)
        {
            std::reverse((char*)&room_idx, (char*)&room_idx + 4);
            std::reverse((char*)&msg_size, (char*)&msg_size + 4);
        }
        return return_t{t, room_idx, msg_size};
    }
    static std::array<std::byte, header_length> ComposeChatHeader(uint32_t ri, const Chat& ch)
    {
        if(ch.text.bytes() > (size_t)UINT32_MAX)
        {
            throw std::runtime_error("Message too long");
        }
        using return_t = decltype(ComposeChatHeader(ri, ch));
        return_t header;
        header[0] = std::bit_cast<std::byte>(MessageType::Communication);
        auto room_span = std::as_writable_bytes(std::span<uint32_t>(&ri,1));
        if(std::endian::native != std::endian::little)
        {
            std::reverse(room_span.begin(), room_span.end());
        }
        std::copy(room_span.begin(), room_span.end(), header.begin()+1);
        uint32_t msg_len = ch.text.bytes();
        auto len_span = std::as_writable_bytes(std::span<uint32_t>(&msg_len, 1));
        if(std::endian::native != std::endian::little)
        {
            std::reverse(len_span.begin(), len_span.end());
        }
        std::copy(len_span.begin(), len_span.end(), header.begin()+5);
        return header;
    }
};