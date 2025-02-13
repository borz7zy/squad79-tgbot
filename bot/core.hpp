#pragma once

#include "../main.hpp"
#include "./mini_ai/helper.h"
#include "./mini_ai/answer_database.h"

class Core : public Singleton<Core>
{
private:
    void removeSystemMessage(const int64_t &chat_id, const std::int32_t &message_id);
    void onAnyMessage(TgBot::Message::Ptr message);
    void addNewMemberChatInDb(TgBot::Message::Ptr &message);

    std::string getCommandName(const std::string &input);

public:
    Core();
    ~Core() override = default;

    std::unique_ptr<TgBot::Bot> bot;

    void getChatMembers(const long long &chat_id, std::vector<std::int64_t> &members);
};