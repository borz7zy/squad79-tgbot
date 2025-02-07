#pragma once

#include "../core.hpp"
#include "command.hpp"
#include "../../database/sqlite_wrapper.hpp"

class InitCmd : public Command, public Singleton<InitCmd>
{
public:
    void execute(TgBot::Message::Ptr message) override
    {
        if (message->chat->type == TgBot::Chat::Type::Group || message->chat->type == TgBot::Chat::Type::Supergroup)
        {
            long long chat_id = message->chat->id;
            long long creator_id = 0;
            std::vector<TgBot::ChatMember::Ptr> admins = Core::Get()->bot->getApi().getChatAdministrators(chat_id);
            for (const auto &admin : admins)
            {
                if (admin->status == "creator")
                {
                    creator_id = admin->user->id;
                }
            }
            if (creator_id == 0)
            {
                rn;
            }
            bool inited = false;
            auto result = SQLiteWrapper::Get()->retrieve("main_init.db", "mainchat:chat_id::int");
            try
            {
                for (const auto &row : result["mainchat"])
                {
                    long long chat_id_db = std::stoll(row.at("chat_id"));
                    if (chat_id_db == chat_id)
                    {
                        Core::Get()->bot->getApi().sendMessage(message->chat->id, "Бот уже был инициализирован здесь =)");
                        inited = true;
                    }
                    else
                    {
                        Core::Get()->bot->getApi().sendMessage(message->chat->id, "Бот уже был инициализирован в другом чате!\nБот может быть привязан только к одному чату =(");
                        inited = true;
                    }
                    rn;
                }
            }
            catch (const std::out_of_range &e)
            {
                Logger::Get()->Log("/init: Ошибка преобразования (out_of_range): %s", e.what());
            }
            catch (const std::invalid_argument &e)
            {
                Logger::Get()->Log("/init: Ошибка преобразования (invalid_argument): %s", e.what());
            }
            catch (const std::exception &e)
            {
                Logger::Get()->Log("/init: Общая ошибка: %s", e.what());
            }

            if (inited == false)
            {
                SQLiteWrapper::Get()->add("main_init.db", {std::string("mainchat:chat_id:int:" + std::to_string(chat_id)),
                                                           std::string("mainchat:creator_id:int:" + std::to_string(creator_id))});
                Core::Get()->bot->getApi().sendMessage(chat_id, "Чат был инициализирован!");
            }
        }
    }
};
