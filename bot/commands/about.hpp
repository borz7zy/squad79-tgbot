#pragma once

#include "command.hpp"

class AboutCmd : public Command, public Singleton<AboutCmd>
{
public:
    void execute(TgBot::Message::Ptr &message) override
    {
        Core::Get()->bot->getApi().sendMessage(message->chat->id, "Source code: \nhttps://github.com/borz7zy/squad79-tgbot\nWriten in C++20");
    }
};
