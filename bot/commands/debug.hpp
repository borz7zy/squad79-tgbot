#pragma once

#include "command.hpp"
#include "../../utils/memory.hpp"

class DebugCmd : public Command, public Singleton<DebugCmd>
{
public:
    void execute(TgBot::Message::Ptr message) override
    {
        bool dbgv = false;
        if (MemCache::Get()->getKeyValue("botDebug", dbgv)) {
            MemCache::Get()->addKeyValue("botDebug", !dbgv);
        }else{
            MemCache::Get()->addKeyValue("botDebug", !dbgv);
        }
        dbgv = !dbgv;
        Core::Get()->bot->getApi().sendMessage(message->chat->id, (dbgv)? "Дебаг активирован!" : "Дебаг деактивирован!");
    }
};
