#pragma once

#include "start.hpp"
#include "about.hpp"
#include "debug.hpp"

void startHandler(TgBot::Message::Ptr message)
{
    StartCmd::Get()->execute(message);
}

void aboutHandler(TgBot::Message::Ptr message)
{
    AboutCmd::Get()->execute(message);
}

void cmdDebugHandler(TgBot::Message::Ptr message)
{
    DebugCmd::Get()->execute(message);
}