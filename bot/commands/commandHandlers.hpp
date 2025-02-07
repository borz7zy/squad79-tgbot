#pragma once

#include "start.hpp"
#include "about.hpp"
#include "debug.hpp"
#include "init.hpp"
#include "permissions.hpp"
#include "admins.hpp"

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

void initHandler(TgBot::Message::Ptr message)
{
    InitCmd::Get()->execute(message);
}

void permissionsHandler(TgBot::Message::Ptr message)
{
    PermissionsCmd::Get()->execute(message);
}

void adminsHandler(TgBot::Message::Ptr message)
{
    AdminsCmd::Get()->execute(message);
}