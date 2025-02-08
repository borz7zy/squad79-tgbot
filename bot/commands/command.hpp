#pragma once

#include "../../main.hpp"
#include "../core.hpp"
#include "../../utils/memory.hpp"
#include "../../database/sqlite_wrapper.hpp"

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
        bool ignoreGroup;
        bool onlyAdmin;
        CommandFunc handler;
    };

    void addCommand(const std::string &commandName, const CommandFunc handler, const bool &ignoreGroup, const bool &onlyAdmin)
    {
        commands.push_back({commandName, ignoreGroup, onlyAdmin, handler});
    }

    void registerCommandsToBot(TgBot::Bot *bot)
    {
        for (const auto &command : commands)
        {
            bot->getEvents().onCommand(command.commandName,
                                       [name = command.commandName,
                                        groupIgnore = command.ignoreGroup,
                                        adminOnly = command.onlyAdmin,
                                        handler = command.handler](TgBot::Message::Ptr message)
                {
                    std::uint64_t admin_id = 0;
                    if(adminOnly)
                    {
                        std::vector<TgBot::ChatMember::Ptr> admins =
                                Core::Get()->bot->getApi().getChatAdministrators(message->chat->id);
                        bool findedAdmin = false;
                        for(const auto &admin : admins){
                            if(admin->user->id && admin->user->id == message->from->id){
                                findedAdmin = true;
                                admin_id = admin->user->id;
                                break;
                            }
                        }
                        if(!findedAdmin){
                            Core::Get()->bot->getApi().sendMessage(message->chat->id, "Команду могут использовать только админы чатов!");
                            rn;
                        }
                    }

                    auto now = std::chrono::system_clock::now();
                    auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                    long long next_ts;
                    std::string key = ("chat::"+std::to_string(message->chat->id)+"::command::"+name+"::timestamp");
                    if(!MemCache::Get()->getKeyValue(key, next_ts)){
                        next_ts = 0;
                    }
                    if(admin_id != 0) next_ts = 0;
                    if(timestamp_ms >= next_ts)
                    {
                        if(groupIgnore)
                        {
                            handler(message);
                            MemCache::Get()->addKeyValue(key, timestamp_ms + (3 * 1000));
                        }else{
                            long long main_chat_id = 0;
                            auto result = SQLiteWrapper::Get()->retrieve(
                                    "main_init.db", "mainchat:chat_id::int");

                            if (result.find("mainchat") != result.end() && !result["mainchat"].empty())
                            {
                                for (const auto &row : result["mainchat"])
                                {
                                    if (row.find("chat_id") != row.end())
                                    {
                                        main_chat_id = std::stoll(row.at("chat_id"));
                                        break;
                                    }else
                                        continue;
                                }
                            }

                            if(message->chat->id == main_chat_id)
                            {
                                if (message->chat->type == TgBot::Chat::Type::Group ||
                                    message->chat->type == TgBot::Chat::Type::Supergroup)
                                {
                                    handler(message);
                                    MemCache::Get()->addKeyValue(key, timestamp_ms + (3 * 1000));
                                } else {
                                    Core::Get()->bot->getApi().sendMessage(
                                            message->chat->id,
                                            "Эта команда может быть использована только в группах или супергруппах.");
                                }
                            }else {
                                Core::Get()->bot->getApi().sendMessage(
                                        message->chat->id,
                                        "Эта команда может быть использована только в чате, в котором инициализирован бот!");
                            }
                        }
                    }
                }
            );
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
