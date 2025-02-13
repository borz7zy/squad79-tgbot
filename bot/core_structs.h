//
// Created by Stas on 13.2.25..
//

#pragma once

#include <cstdint>
#include <map>
#include <variant>

enum eventTypes : uint8_t
{
    EVENT_TYPE_UNKNOWN = 0xFF,
    EVENT_TYPE_MESSAGE = 0x0,
    EVENT_TYPE_COMMAND
};

struct eventInfo
{
    int32_t event_id;
    int64_t chat_id;
    std::variant<std::string, int64_t> sender_id;
    eventTypes event_type;
    std::variant<std::unordered_map<uint32_t, std::string>, std::string> event_data;
};
