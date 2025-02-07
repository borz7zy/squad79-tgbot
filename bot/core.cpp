#include "core.hpp"
#include "../utils/logger.hpp"
#include "./commands/commandHandlers.hpp"

Core::Core(){
    const char* tokenEnv = std::getenv("TGBOT_TOKEN");
    if (tokenEnv == nullptr) {
        Logger::Get()->Log("Error: Token not found in environment variable TGBOT_TOKEN");
        exit(1);
    }

    std::string token(tokenEnv);
    bot = std::make_unique<TgBot::Bot>(token);

    // bot->getEvents().onCommand("start", [this](TgBot::Message::Ptr message) {
    //     onStartCommand(message);
    // });
    CommandManager::Get()->addCommand("start", startHandler);
    CommandManager::Get()->addCommand("about", aboutHandler);

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
    if (CommandManager::Get()->commandExists(getCommandName(message->text))) {
        Logger::Get()->Log("User %s use command: %s", message->from->username.c_str(), message->text.c_str());
        rn;
    }
    Logger::Get()->Log("User %s send text: %s", message->from->username.c_str(), message->text.c_str());
    // bot->getApi().sendMessage(message->chat->id, "Your message is: " + message->text);
}