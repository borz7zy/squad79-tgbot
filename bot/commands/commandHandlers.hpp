#pragma once

#include "start.hpp"
#include "about.hpp"

void startHandler(TgBot::Message::Ptr message)
{
    StartCmd helloCmd;
    helloCmd.execute(message);
}

void aboutHandler(TgBot::Message::Ptr message)
{
    AboutCmd about;
    about.execute(message);
}