#pragma once

#include "command.hpp"
#include "../../database/sqlite_wrapper.hpp"


class StartCmd : public Command, public Singleton<StartCmd>
{
public:
    void execute(TgBot::Message::Ptr &message) override
    {
        auto result = SQLiteWrapper::Get()->retrieve("main_init.db", "mainchat:chat_id::int");
        bool sended = false;
        if (result.find("mainchat") != result.end() && !result["mainchat"].empty())
        {
            for (const auto &row : result["mainchat"])
            {
                if (row.find("chat_id") != row.end())
                {
                    long long chatIdDb = std::stoll(row.at("chat_id"));

                    if(chatIdDb != message->chat->id){
                        sended = true;

                        Core::Get()->bot->getApi().sendMessage(message->chat->id, "Рад тебя видеть!\n\n"
                        "Это бот беседы SQUAD79, соответственно всё что в нём есть - работает исключительно в этой беседе.\n\n"
                        "Исходник можно найти в команде /about .");

                        break;
                    }else{
                        sended = true;

                        Core::Get()->bot->getApi().sendMessage(message->chat->id, "Чат уже инициализирован!\n\n"
                        "Для помощи введите /help");

                        break;
                    }
                }
            }
        }
        if(!sended){
            Core::Get()->bot->getApi().sendMessage(message->chat->id, "Рад тебя видеть!\n\n"
            "Это бот беседы SQUAD79, соответственно всё что в нём есть - работает исключительно в этой беседе.\n\n"
            "Исходник можно найти в команде /about .");
        }
    }
};
