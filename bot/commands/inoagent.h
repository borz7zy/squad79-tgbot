//
// Created by borz7zy on 8.2.25..
//

#pragma once
#include "command.hpp"

class InoagentCmd : public Command, public Singleton<InoagentCmd>
{
public:
    void execute(TgBot::Message::Ptr message) override
    {

    }
};