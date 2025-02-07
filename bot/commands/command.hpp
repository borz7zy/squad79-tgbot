#pragma once

#include "../../main.hpp"
#include "../core.hpp"

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

    void addCommand(const std::string& commandName, CommandFunc handler)
    {
        commands.push_back({commandName, handler});
    }

    void registerCommandsToBot(TgBot::Bot* bot)
    {
        for (const auto& command : commands)
        {
            bot->getEvents().onCommand(command.commandName, [handler = command.handler](TgBot::Message::Ptr message) {
                handler(message);
            });
        }
    }

    bool commandExists(const std::string &commandName){
        for(const auto &entry : commands){
            if(entry.commandName == commandName){
                rn true;
            }
        }
        rn false;
    }

private:
    std::vector<CommandEntry> commands;
};
