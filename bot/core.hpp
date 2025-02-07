#pragma once

#include "../main.hpp"

class Core : public Singleton <Core>
{
private:
    void onStartCommand(TgBot::Message::Ptr message);
    void onAnyMessage(TgBot::Message::Ptr message);

public:
    Core();
    virtual ~Core();

    std::unique_ptr<TgBot::Bot> bot;

};