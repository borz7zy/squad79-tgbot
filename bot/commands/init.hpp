#pragma once

#include "../core.hpp"
#include "command.hpp"
#include "../../database/dbcontroller.hpp"

class InitCmd : public Command, public Singleton<InitCmd>
{
public:
    void execute(TgBot::Message::Ptr message) override
    {
        if (message->chat->type == TgBot::Chat::Type::Group || message->chat->type == TgBot::Chat::Type::Supergroup)
        {
            long long chat_id = message->chat->id;

            std::string pattern = R"(chat::[-]?\d+::main)";
            auto results = Database::Get()->searchKeys<int>(pattern);

            std::regex chatIdRegex(R"(chat::(-?\d+)::main)");

            for (const auto &[key, value] : results)
            {

                if (value)
                {
                    std::smatch match;
                    if (std::regex_search(key, match, chatIdRegex) && match.size() > 1)
                    {
                        long long extractedChatId = std::stoll(match.str(1));
                        Logger::Get()->Log("%s %s", std::to_string(extractedChatId).c_str(), std::to_string(chat_id).c_str());
                        if (extractedChatId == chat_id)
                        {
                            Core::Get()->bot->getApi().sendMessage(message->chat->id, "Бот уже был инициализирован здесь =)");
                        }
                        else
                        {
                            Core::Get()->bot->getApi().sendMessage(message->chat->id, "Бот уже был инициализирован в другом чате!\nБот может быть привязан только к одному чату =(");
                        }
                    }
                    rn;
                }
            }

            std::vector<TgBot::ChatMember::Ptr> admins = Core::Get()->bot->getApi().getChatAdministrators(chat_id);
            for (const auto &admin : admins)
            {
                if (admin->user->username != Core::Get()->bot->getApi().getMe()->username.c_str())
                {
                    std::string key = ("chat::" + std::to_string(chat_id) + "::admin::" + std::to_string(admin->user->id));
                    Database::Get()->put(key, true);
                }
                Database::Get()->put(std::string("chat::" + std::to_string(chat_id) + "::main"), true);
            }
            Core::Get()->bot->getApi().sendMessage(message->chat->id, "Чат был инициализирован!");
        }
    }
};
