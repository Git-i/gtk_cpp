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
    DateTimeNetwork ToNetwork()
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
struct Message
{
    Glib::ustring text;
    DateTime time;
    uint32_t user;
    std::vector<uint64_t> attachments;
    std::vector<std::byte> IntoBytes()
    {
        std::vector<std::byte> returnVal;
        uint32_t user_le = user;
        auto net_time = time.ToNetwork();
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
        auto text_size = text.bytes();
        returnVal.resize(old_size + text_size);
        text.copy(reinterpret_cast<char*>(returnVal.data()) + old_size, text_size);
        //TODO attachments;
        return returnVal;
    }
    static Message FromBytes(std::span<std::byte> bytes, uint32_t size)
    {
        Message msg;
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
        msg.text = Glib::ustring(reinterpret_cast<char*>(bytes.data()) + 8, bytes.size() - 4);
        return msg;
    }
};