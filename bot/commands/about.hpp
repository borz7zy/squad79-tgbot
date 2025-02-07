#pragma once

#include "command.hpp"


class AboutCmd : public Command
{
public:
    void execute(TgBot::Message::Ptr message) override
    {
        Core::Get()->bot->getApi().sendMessage(message->chat->id, "Source code: \nhttps://github.com/borz7zy/squad79-tgbot\n");
    }
};
