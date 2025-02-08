#include "core.hpp"
#include "../utils/logger.hpp"
#include "./commands/commandHandlers.hpp"
#include "../utils/memory.hpp"
#include "../utils/escapeMarkdownV2.hpp"

Core::Core()
{
    const char *tokenEnv = std::getenv("TGBOT_TOKEN");
    if (tokenEnv == nullptr)
    {
        Logger::Get()->Log("Error: Token not found in environment variable TGBOT_TOKEN");
        exit(1);
    }

    std::string token(tokenEnv);
    bot = std::make_unique<TgBot::Bot>(token);

    CommandManager::Get()->addCommand("start", startHandler, true, false);
    CommandManager::Get()->addCommand("about", aboutHandler, true, false);
    CommandManager::Get()->addCommand("debug", cmdDebugHandler, false, true);
    CommandManager::Get()->addCommand("init", initHandler, false, true);
    CommandManager::Get()->addCommand("permissions", permissionsHandler, false, false);
    CommandManager::Get()->addCommand("admins", adminsHandler, false, false);
    CommandManager::Get()->addCommand("inoagent", inoagentHandler, false, false);

    CommandManager::Get()->registerCommandsToBot(bot.get());

    bot->getEvents().onAnyMessage([this](TgBot::Message::Ptr message)
        {
            onAnyMessage(message);
        }
    );

    Logger::Get()->Log("Bot %s started!", bot->getApi().getMe()->username.c_str());
}

Core::~Core()
{
}

std::string Core::getCommandName(const std::string &input)
{
    size_t spacePos = input.find(' ');
    if (spacePos != std::string::npos)
    {
        rn input.substr(1, spacePos - 1);
    }
    else
    {
        rn input.substr(1);
    }
}

void Core::removeSystemMessage(long long &chat_id, std::int32_t &message_id)
{
    try
    {
        bot->getApi().deleteMessage(chat_id, message_id);
    }
    catch (TgBot::TgException &e)
    {
        bot->getApi().sendMessage(chat_id, "Ошибка удаления системного сообщения!"
                                           "\nВозможно у бота недостаточно привелегий =(");
        Logger::Get()->Log("Ошибка удаления системного сообщения! Недостаточно прав");
    }
}

void Core::onAnyMessage(TgBot::Message::Ptr &message)
{
    long long main_chat_id = 0;
    auto result = SQLiteWrapper::Get()->retrieve(
            "main_init.db", "mainchat:chat_id::int");

    if (result.find("mainchat") != result.end() && !result["mainchat"].empty())
    {
        for (const auto &row : result["mainchat"])
        {
            if (row.find("chat_id") != row.end())
            {
                main_chat_id = std::stoll(row.at("chat_id"));
                break;
            }else
                continue;
        }
    }
    if(message->chat->id != main_chat_id)
        rn;

    std::string response;
    if (!message->newChatTitle.empty())
    {
        response = "Название чата изменено на: " + message->newChatTitle;
        removeSystemMessage(message->chat->id, message->messageId);
    }
    else if (!message->newChatPhoto.empty())
    {
        response = "Фото чата обновлено.";
    }
    else if (message->deleteChatPhoto)
    {
        response = "Фото чата удалено.";
    }
    else if (message->groupChatCreated)
    {
        response = "Группа создана.";
    }
    else if (message->supergroupChatCreated)
    {
        response = "Супергруппа создана.";
    }
    else if (message->channelChatCreated)
    {
        response = "Канал создан.";
    }
    else if (message->migrateToChatId != 0)
    {
        response = "Группа обновлена до супергруппы.";
    }
    else if (message->migrateFromChatId != 0)
    {
        response = "Супергруппа понижена до группы.";
    }
    else if (message->pinnedMessage != nullptr)
    {
        response = "Сообщение закреплено.";
    }
    else if (!message->newChatMembers.empty())
    {
        response = "Добавлен новый участник: ";
        removeSystemMessage(message->chat->id, message->messageId);
        for (const auto &member : message->newChatMembers)
        {
            response += member->firstName + " ";

            try
            {
                auto permissions = std::make_shared<TgBot::ChatPermissions>();
                permissions->canSendMessages = false;       // Запрет отправки сообщений
                permissions->canSendAudios = false;         // Запрет отправки аудио
                permissions->canSendDocuments = false;      // Запрет отправки файлов
                permissions->canSendPhotos = false;         // Запрет отправки фото
                permissions->canSendVideos = false;         // Запрет отправки видео
                permissions->canSendVideoNotes = false;     // Запрет отправки видео-заметок
                permissions->canSendVoiceNotes = false;     // Запрет отправки голосовых сообщений
                permissions->canSendPolls = false;          // Запрет отправки опросов
                permissions->canSendOtherMessages = false;  // Запрет отправки стикеров, игр и инлайн-ботов
                permissions->canAddWebPagePreviews = false; // Запрет вставки ссылок с превью

                bot->getApi().restrictChatMember(
                    message->chat->id,
                    member->id,
                    permissions,
                    std::numeric_limits<std::int32_t>::max());

                std::ostringstream ss;
                ss << "Пользователь ";
                ss << "[" << escapeMarkdownV2(member->firstName) <<
                "](tg://user?id=" + std::to_string(member->id) + ")";
                ss << escapeMarkdownV2(" присоединился и автоматически был ограничен "
                                       "в возможностях отправки сообщений.");
                bot->getApi().sendMessage(message->chat->id, ss.str(),
                                          nullptr, nullptr, nullptr,
                                          "MarkdownV2", true);

                Logger::Get()->Log("Новый пользователь был ограничен на сообщения: %s",
                                   std::to_string(member->id).c_str());
            }
            catch (TgBot::TgException &e)
            {
                Logger::Get()->Log("Ошибка при ограничении пользователя: %s", e.what());
            }
        }
    }
    else if (message->leftChatMember != nullptr)
    {
        response = "Участник покинул чат: " + message->leftChatMember->firstName;
        removeSystemMessage(message->chat->id, message->messageId);
    }

    if (!response.empty())
    {
        Logger::Get()->Log(response.c_str());
    }

    if (message->text.length() <= 0)
        rn;

    bool dbgv = false;
    MemCache::Get()->getKeyValue("botDebug", dbgv);

    std::string chat_type = "";
    if (dbgv)
    {
        switch (message->chat->type)
        { // Private, Group, Supergroup, Channel
        case TgBot::Chat::Type::Private:
        {
            chat_type = "Личка";
            break;
        }
        case TgBot::Chat::Type::Group:
        {
            chat_type = "Чат";
            break;
        }
        case TgBot::Chat::Type::Supergroup:
        {
            chat_type = "Суперчат";
            break;
        }
        case TgBot::Chat::Type::Channel:
        {
            chat_type = "Канал";
            break;
        }
        }
    }
    if (CommandManager::Get()->commandExists(getCommandName(message->text)))
    {
        if (dbgv)
        {
            if (message->chat->type != TgBot::Chat::Type::Channel)
            {
                Logger::Get()->Log("User %s use command: %s", message->from->username.c_str(), message->text.c_str());
                bot->getApi().sendMessage(message->chat->id, "Юзер @" + message->from->username + "( " +
                std::to_string(message->from->id) + " ) ввел команду /" + getCommandName(message->text) +
                " , тип чата: " + chat_type + ".");
            }
        }
        rn;
    }
    if (dbgv)
    {
        if (message->chat->type != TgBot::Chat::Type::Channel)
        {
            Logger::Get()->Log("User %s send text: %s", message->from->username.c_str(), message->text.c_str());
            bot->getApi().sendMessage(message->chat->id,
                                      "Юзер @" + message->from->username + "( " +
                                      std::to_string(message->from->id) + " ) отправил сообщение в чате " +
                                      std::to_string(message->chat->id) + ", тип чата: " + chat_type +
                                      ". Сообщение:\n\n" + message->text);
        }
    }
}