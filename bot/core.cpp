#include "core.hpp"
#include "../utils/logger.hpp"
#include "./commands/commandHandlers.hpp"
#include "../utils/memory.hpp"

Core::Core(){
    const char* tokenEnv = std::getenv("TGBOT_TOKEN");
    if (tokenEnv == nullptr) {
        Logger::Get()->Log("Error: Token not found in environment variable TGBOT_TOKEN");
        exit(1);
    }

    std::string token(tokenEnv);
    bot = std::make_unique<TgBot::Bot>(token);

    CommandManager::Get()->addCommand("start", startHandler);
    CommandManager::Get()->addCommand("about", aboutHandler);
    CommandManager::Get()->addCommand("debug", cmdDebugHandler);

    CommandManager::Get()->registerCommandsToBot(bot.get());

    bot->getEvents().onAnyMessage([this](TgBot::Message::Ptr message) {
        onAnyMessage(message);
    });

    Logger::Get()->Log("Bot %s started!", bot->getApi().getMe()->username.c_str());
}

Core::~Core() {
}

void Core::onStartCommand(TgBot::Message::Ptr message) {
    bot->getApi().sendMessage(message->chat->id, "Hi!");
}

std::string Core::getCommandName(const std::string &input){
    size_t spacePos = input.find(' ');
    if(spacePos != std::string::npos){
        rn input.substr(1, spacePos-1);
    }else{
        rn input.substr(1);
    }
}

void Core::onAnyMessage(TgBot::Message::Ptr message) {
    if(message->text.length() <= 0)
        rn;

    bool dbgv = false;
    MemCache::Get()->getKeyValue("botDebug", dbgv);
    
    std::string chat_type = "";
    if(dbgv) {
        switch(message->chat->type){//Private, Group, Supergroup, Channel
            case TgBot::Chat::Type::Private:{
                chat_type = "Личка";
                break;
            }
            case TgBot::Chat::Type::Group:{
                chat_type = "Чат";
                break;
            }
            case TgBot::Chat::Type::Supergroup:{
                chat_type = "Суперчат";
                break;
            }
            case TgBot::Chat::Type::Channel:{
                chat_type = "Канал";
                break;
            }
        }
    }
    if (CommandManager::Get()->commandExists(getCommandName(message->text))) {
        if(dbgv){
            if(message->chat->type != TgBot::Chat::Type::Channel) {
                Logger::Get()->Log("User %s use command: %s", message->from->username.c_str(), message->text.c_str());
                bot->getApi().sendMessage(message->chat->id, "Юзер @"+message->from->username+"( "+std::to_string(message->from->id)
                +" ) ввел команду /"+getCommandName(message->text)+" , тип чата: "+chat_type+".");
            }
        }
        rn;
    }
    if(dbgv){
        if(message->chat->type != TgBot::Chat::Type::Channel) {
            Logger::Get()->Log("User %s send text: %s", message->from->username.c_str(), message->text.c_str());
            bot->getApi().sendMessage(message->chat->id, 
            "Юзер @"+message->from->username+"( "+std::to_string(message->from->id)+" ) отправил сообщение в чате "
            +std::to_string(message->chat->id)+", тип чата: "+chat_type+". Сообщение:\n\n"+message->text);
        }
    }
}