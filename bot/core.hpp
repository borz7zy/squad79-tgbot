#pragma once

#include "../main.hpp"

class Core : public Singleton<Core>
{
private:
    void removeSystemMessage(const long long &chat_id, const std::int32_t &message_id);
    void onAnyMessage(TgBot::Message::Ptr &message);
    void addNewMemberChatInDb(TgBot::Message::Ptr &message);

    std::string getCommandName(const std::string &input);

public:
    Core();
    ~Core() override = default;

    std::unique_ptr<TgBot::Bot> bot;

    void getChatMembers(const long long &chat_id, std::vector<std::int32_t> &members);
};