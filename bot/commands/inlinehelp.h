//
// Created by Stas on 11.2.25..
//

#pragma once

#include "command.hpp"

class InlinehelpCmd : public Command, public Singleton<InlinehelpCmd>
{
public:
    void execute(TgBot::Message::Ptr &message) override
    {
        
    }
};
