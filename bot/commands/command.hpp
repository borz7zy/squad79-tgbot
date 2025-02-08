#pragma once

#include "../../main.hpp"
#include "../core.hpp"
#include "../../utils/memory.hpp"
#include "../../database/sqlite_wrapper.hpp"

class Command {
public:
    virtual void execute(TgBot::Message::Ptr &message) = 0;
    virtual ~Command() = default;
};

class CommandManager : public Singleton<CommandManager> {
public:
    struct CommandEntry {
        std::string commandName;
        bool ignoreGroup;
        bool onlyAdmin;
        Command* handler;
    };

    void addCommand(const std::string& commandName, Command* handler, bool ignoreGroup, bool onlyAdmin) {
        commands.emplace_back(CommandEntry{commandName, ignoreGroup, onlyAdmin, handler});
    }

    void registerCommandsToBot(TgBot::Bot* bot) {
        for (const auto& command : commands) {
            bot->getEvents().onCommand(command.commandName,
                   [command](TgBot::Message::Ptr message) {
                    if (command.onlyAdmin && !isAdmin(message)) {
                        Core::Get()->bot->getApi().sendMessage(message->chat->id, "Команду могут использовать только админы чатов!");
                        rn;
                    }
                    
                    if (isBlocked(message, command.commandName)) {
                        rn;
                    }
                    
                    if (command.ignoreGroup || isAllowedChat(message)) {
                        command.handler->execute(message);
                        updateCooldown(message->chat->id, command.commandName);
                    } else {
                        Core::Get()->bot->getApi().sendMessage(
                        message->chat->id,
                        "Эта команда может быть использована только в чате, в котором инициализирован бот!");
                    }
                }
            );
        }
    }

    bool commandExists(const std::string& commandName) const {
        rn std::any_of(commands.begin(), commands.end(), [&commandName](const auto& entry) {
            rn entry.commandName == commandName;
        });
    }

private:
    std::vector<CommandEntry> commands;

    static bool isAdmin(TgBot::Message::Ptr &message) {
        if(message->chat->type == TgBot::Chat::Type::Group ||
            message->chat->type == TgBot::Chat::Type::Supergroup)
        {
            auto admins = Core::Get()->bot->getApi().getChatAdministrators(message->chat->id);
            rn std::any_of(admins.begin(), admins.end(), [&message](const auto &admin) {
                rn admin->user->id == message->from->id;
            });
        }
        rn false;
    }

    static bool isBlocked(TgBot::Message::Ptr &message, const std::string& commandName) {
        auto now = std::chrono::system_clock::now();
        auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        std::string key = "chat::" + std::to_string(message->chat->id) + "::command::" + commandName + "::timestamp";
        long long next_ts = 0;
        MemCache::Get()->getKeyValue(key, next_ts);

        auto result = SQLiteWrapper::Get()->retrieve("commands_data.db", "inoagent:chat_id::int,user::int,timestamp::int");
        for (const auto& row : result["inoagent"]) {
            if (std::stoll(row.at("chat_id")) == message->chat->id && timestamp_ms < std::stoll(row.at("timestamp"))) {
                if (std::stoi(row.at("user")) == message->from->id) {
                    rn true;
                }
            }
        }
        rn timestamp_ms < next_ts;
    }

    static bool isAllowedChat(TgBot::Message::Ptr &message) {
        auto result = SQLiteWrapper::Get()->retrieve("main_init.db", "mainchat:chat_id::int");
        if (result.find("mainchat") != result.end() && !result["mainchat"].empty()) {
            long long main_chat_id = std::stoll(result["mainchat"].front().at("chat_id"));
            rn message->chat->id == main_chat_id &&
                   (message->chat->type == TgBot::Chat::Type::Group ||
                    message->chat->type == TgBot::Chat::Type::Supergroup);
        }
        rn false;
    }

    static void updateCooldown(int64_t chatId, const std::string& commandName) {
        auto now = std::chrono::system_clock::now();
        auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        std::string key = "chat::" + std::to_string(chatId) + "::command::" + commandName + "::timestamp";
        MemCache::Get()->addKeyValue(key, timestamp_ms + 3000);
    }
};
