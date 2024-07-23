#pragma once
#include "glibmm/ustring.h"
#include <bit>
#include <cstddef>
#include <ctime>
#include <span>
#include <cstdint>
#include <vector>
#include <algorithm>
enum class MessageType : uint8_t
{
    Invalid = 0,
    Communication = 1,
    LeaveRequest = 2,
    UserDetails = 3,
    UserId = 4
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
    template<typename Iterator>
    static void AppendU32(uint32_t value, Iterator pos)
    {
        auto value_le = std::as_writable_bytes(std::span<uint32_t>(&value, 1));
        if(std::endian::native != std::endian::little)
        {
            std::reverse(value_le.begin(), value_le.end());
        }
        std::copy(value_le.begin(), value_le.end(), pos);
    }
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
    static std::vector<std::byte> MakeChatForForwarding(uint32_t ri, const Chat& ch)
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
        std::array<std::byte, header_length> header;
        header[0] = std::bit_cast<std::byte>(MessageType::Communication);
        AppendU32(ri, header.begin()+1);
        uint32_t msg_len = ch.text.bytes();
        AppendU32(msg_len, header.begin()+5);
        return header;
    }
    static std::vector<std::byte> MakeUserDetailForForwarding(const Glib::ustring& name, uint32_t id)
    {
        std::vector<std::byte> value;
        auto header = ComposeUserDetailHeader(id, name.bytes());
        auto body = ComposeUserDetailBody(name);
        value.insert(value.begin(), header.begin(), header.end());
        value.insert(value.begin(), body.begin(), body.end());
        return value;
    }
    static std::array<std::byte, header_length> ComposeUserDetailHeader(uint32_t user_id, uint32_t name_len)
    {
        std::array<std::byte, header_length> val;
        val[0] = std::bit_cast<std::byte>(MessageType::UserDetails);
        AppendU32(user_id, val.begin()+1);
        AppendU32(name_len, val.begin()+5);
        return val;
    }
    static std::vector<std::byte> ComposeUserDetailBody(const Glib::ustring& name)
    {
        std::vector<std::byte> value;
        value.resize(name.bytes());
        name.copy((char*)value.data(), value.size());
        return value;
    }
    static Glib::ustring DecomposeUserDetailBody(std::span<std::byte> body)
    {
        return Glib::ustring((char*)body.data());
    }
    static std::array<std::byte, header_length> ComposeUserId(uint32_t id)
    {
        std::array<std::byte, Message::header_length> val;
        uint32_t user_id_le = id;
        if(std::endian::native != std::endian::little) std::reverse((char*)&user_id_le, (char*)&user_id_le+4);
        val[0] = std::bit_cast<std::byte>(MessageType::UserId);
        std::copy(reinterpret_cast<std::byte*>(&user_id_le),reinterpret_cast<std::byte*>(&user_id_le) + 4, val.begin() + 1);
        return val;
    }
};