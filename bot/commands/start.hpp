#pragma once

#include "command.hpp"


class StartCmd : public Command
{
public:
    void execute(TgBot::Message::Ptr message) override
    {
        Core::Get()->bot->getApi().sendMessage(message->chat->id, "Рад тебя видеть!\n\
        \nЭто бот беседы SQUAD79, соответственно всё что в нём есть - работает исключительно в этой беседе.\
        \n\nИсходник можно найти в команде /about .");
    }
};
