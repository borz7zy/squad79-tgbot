#pragma once

#include "../../main.hpp"
#include "../core.hpp"
#include "../../utils/memory.hpp"

class Command
{
public:
    virtual void execute(TgBot::Message::Ptr message) = 0;
    virtual ~Command() = default;
};

class CommandManager : public Singleton<CommandManager>
{
public:
    using CommandFunc = std::function<void(TgBot::Message::Ptr message)>;

    struct CommandEntry
    {
        std::string commandName;
        CommandFunc handler;
    };

    void addCommand(const std::string &commandName, CommandFunc handler)
    {
        commands.push_back({commandName, handler});
    }

    void registerCommandsToBot(TgBot::Bot *bot)
    {
        for (const auto &command : commands)
        {
            bot->getEvents().onCommand(command.commandName, [name = command.commandName, handler = command.handler](TgBot::Message::Ptr message)
                                       {
                auto now = std::chrono::system_clock::now();
                auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                long long next_ts;
                std::string key = ("chat::"+std::to_string(message->chat->id)+"::command::"+name+"::timestamp");
                MemCache::Get()->getKeyValue(key, next_ts);
                if(timestamp_ms >= next_ts)
                {
                    handler(message);
                    MemCache::Get()->addKeyValue(key, timestamp_ms+(3*1000));
                } });
        }
    }

    bool commandExists(const std::string &commandName)
    {
        for (const auto &entry : commands)
        {
            if (entry.commandName == commandName)
            {
                rn true;
            }
        }
        rn false;
    }

private:
    std::vector<CommandEntry> commands;
};
