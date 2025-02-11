#pragma once

#include "../main.hpp"
#include "./mini_ai/helper.h"
#include "./mini_ai/answer_database.h"

static constexpr size_t MAX_HISTORY_MESSAGES_SIZE = 50;

class Core : public Singleton<Core>
{
private:
    void removeSystemMessage(const long long &chat_id, const std::int32_t &message_id);
    void onAnyMessage(TgBot::Message::Ptr &message);
    void addNewMemberChatInDb(TgBot::Message::Ptr &message);

    std::string getCommandName(const std::string &input);

    static std::string getTrollDoxStr(const unsigned short int i, const TgBot::InlineQuery::Ptr &query);

    std::deque<Message> message_history;

public:
    Core();
    ~Core() override = default;

    std::unique_ptr<TgBot::Bot> bot;

    void getChatMembers(const long long &chat_id, std::vector<std::int32_t> &members);
};