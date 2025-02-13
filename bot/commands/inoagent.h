//
// Created by borz7zy on 8.2.25..
//

#pragma once
#include "command.hpp"
#include "../../database/sqlite_wrapper.hpp"
#include "../../utils/escapeMarkdownV2.hpp"
#include "../core.hpp"

#include <random>
#include <algorithm>

class InoagentCmd : public Command, public Singleton<InoagentCmd>
{
private:
    SQLiteWrapper *db = SQLiteWrapper::Get();
    void start(){
        if (!db)
        {
            throw std::runtime_error("Не удалось получить экземпляр SQLiteWrapper");
        }

        db->execute("commands_data.db",
                    "CREATE TABLE IF NOT EXISTS inoagent (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "chat_id INTEGER, user INTEGER, timestamp INTEGER)");
    }

public:
    void execute(TgBot::Message::Ptr &message) override
    {

        auto result = db->retrieve("commands_data.db", "inoagent:chat_id::int,user::int,timestamp::int");
        auto now = std::chrono::system_clock::now();
        long long timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

        unsigned short inoagent_get = 0;

        for (const auto &row : result["inoagent"])
        {
            if(timestamp_ms >= std::stoll(row.at("timestamp")))
            {
                inoagent_get = 1;
            }
            if (std::stoll(row.at("chat_id")) == message->chat->id && timestamp_ms < std::stoll(row.at("timestamp")))
            {
                std::ostringstream ss;
                auto s = Core::Get()->bot->getApi().getChatMember(message->chat->id, std::stol(row.at("user")));
                ss << "👤 "
                   << "[" + escapeMarkdownV2(s->user->firstName) + "](tg://user?id=" + std::to_string(std::stol(row.at("user"))) + ")";
                ss << escapeMarkdownV2(
                        " уже иноагент!\nСтатус иноагента не позволяет пользователю использовать команды!");

                Core::Get()->bot->getApi().sendMessage(message->chat->id,
                                                       ss.str(), nullptr, nullptr,
                                                       nullptr, "MarkdownV2",true);
                inoagent_get = 2;
            }
        }

        std::vector<std::int64_t> members;
        Core::Get()->getChatMembers(message->chat->id, members);

        if (members.empty()) {
            Core::Get()->bot->getApi().sendMessage(message->chat->id, "Какая то фантомная беседа...");
            rn;
        }

        if(inoagent_get != 2) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uint64_t randomMember = members[std::uniform_int_distribution<size_t>(0, members.size() - 1)(gen)];
            long long next_ts = timestamp_ms + 86400000;

            auto s = Core::Get()->bot->getApi().getChatMember(message->chat->id, randomMember);
            std::ostringstream ss;
            ss << "👤 "
               << "[" + escapeMarkdownV2(s->user->firstName) + "](tg://user?id=" + std::to_string(randomMember) + ")";
            ss << escapeMarkdownV2(
                    " стал иноагентом на сутки!\nСтатус иноагента не позволяет пользователю использовать команды!");

            Core::Get()->bot->getApi().sendMessage(message->chat->id,
                                                   ss.str(), nullptr, nullptr,
                                                   nullptr, "MarkdownV2",true);
            if(inoagent_get == 1){
                db->execute("commands_data.db",
                            "UPDATE inoagent SET timestamp=" + std::to_string(next_ts) +
                            " WHERE user=" + std::to_string(randomMember) +
                            " AND chat_id=" + std::to_string(message->chat->id) + ";");
            }else{
                db->add("commands_data.db", {
                        "inoagent:chat_id:int:" + std::to_string(message->chat->id) +
                        ",inoagent:user:int:" + std::to_string(randomMember) +
                        ",inoagent:timestamp:int:" + std::to_string(next_ts)});
            }
        }
    }
};
