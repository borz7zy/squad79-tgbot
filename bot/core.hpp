#pragma once

#include "../main.hpp"

class Core : public Singleton <Core>
{
private:
    void onStartCommand(TgBot::Message::Ptr message);
    void onAnyMessage(TgBot::Message::Ptr message);

    std::string getCommandName(const std::string &input);

public:
    Core();
    virtual ~Core();

    std::unique_ptr<TgBot::Bot> bot;

};