#include "core.hpp"
#include "../utils/logger.hpp"
#include "./commands/commandHandlers.hpp"
#include "../utils/memory.hpp"
#include "../utils/escapeMarkdownV2.hpp"
#include "../database/sqlite_wrapper.hpp"
#include "./mini_ai/helper.h"
#include "./mini_ai/answer_database.h"

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

    bot->getEvents().onInlineQuery([this](TgBot::InlineQuery::Ptr query){
        std::vector<TgBot::InlineQueryResult::Ptr> results;

        auto result = std::make_shared<TgBot::InlineQueryResultArticle>();
        result->id = "1";

        if(query->query.empty())
            result->title = "Напиши ник пользователя которого будем доксить";
        else
            result->title = std::format("Ник пользователя для докса: {}", query->query);

        auto messageContent = std::make_shared<TgBot::InputTextMessageContent>();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<unsigned short> dist(0, 12);

        unsigned short int random_number = dist(gen);

        messageContent->messageText = getTrollDoxStr(random_number, query);

        result->inputMessageContent = messageContent;
        results.push_back(result);

        bot->getApi().answerInlineQuery(query->id, results, 0);
    });

    CommandManager::Get()->addCommand("start", StartCmd::Get(), true, false);
    CommandManager::Get()->addCommand("about", AboutCmd::Get(), true, false);
    CommandManager::Get()->addCommand("debug", DebugCmd::Get(), false, true);
    CommandManager::Get()->addCommand("init", InitCmd::Get(), false, true);
    CommandManager::Get()->addCommand("permissions", PermissionsCmd::Get(), false, false);
    CommandManager::Get()->addCommand("admins", AdminsCmd::Get(), false, false);
    CommandManager::Get()->addCommand("inoagent", InoagentCmd::Get(), false, false);
    CommandManager::Get()->addCommand("inlinehelp", InlinehelpCmd::Get(), true, false);

    CommandManager::Get()->registerCommandsToBot(bot.get());

    bot->getEvents().onAnyMessage([this](TgBot::Message::Ptr message)
        {
            onAnyMessage(message);
        }
    );

    Logger::Get()->Log("Bot %s started!", bot->getApi().getMe()->username.c_str());
}

std::string Core::getTrollDoxStr(const unsigned short int i, const TgBot::InlineQuery::Ptr &query)
{
    std::string troll_dox_str;

    if(i < 0 || i > 12)
        rn std::string("0xFFCCFD");

    switch(i){
        case 0:{
            troll_dox_str = std::string("Хэй, чел... Не надо!");
            break;
        }
        case 1:{
            troll_dox_str = std::string("Прекрати!");
            break;
        }
        case 2:{
            troll_dox_str = std::string("Ты че крыса, мы можем тебя быстро поймать!");
            break;
        }
        case 4:{
            troll_dox_str = std::string("Не вынуждай меня делать плохие вещи!");
            break;
        }
        case 6:{
            troll_dox_str = std::string("Может ты прекратишь?");
            break;
        }
        case 8:{
            troll_dox_str = std::string("Прекращай заниматься фигнёй!!!");
            break;
        }
        case 10:{
            troll_dox_str = std::string("Прекращай!!!!");
            break;
        }
        case 12:{
            troll_dox_str = std::string("Ты меня разозлил, жди наряд около дома!");
            break;
        }
    }
    if ((i) == 3) {
        if(query->from->username.empty()){
            rn getTrollDoxStr(7, query);
        }
        rn std::format("Я уже знаю твой юзернейм, @{}.", (!query->from->username.empty()) ? query->from->username : "");
    }
    else if ((i) == 5) {
        rn std::format("Я уже нашёл твой ID: {} =)", query->from->id);
    }
    else if ((i) == 7) {
        rn std::format("Твое имя {} {}, остановись!",
                                    (!query->from->firstName.empty()) ? query->from->firstName : "",
                                    (!query->from->lastName.empty()) ? query->from->lastName : "");
    }
    else if (i == 9) {
        rn std::format("Твой язык {}, думаю дальше мне не следует говорить о том, что может с тобой произойти!",
                                    (!query->from->languageCode.empty()) ? query->from->languageCode : "");
    }
    else if ((i) == 11) {
        rn std::format("Даю последний шанс исправиться, {} {}!",
                           (!query->from->firstName.empty()) ? query->from->firstName : "",
                           (!query->from->lastName.empty()) ? query->from->lastName : "");
    }
    else {
        rn troll_dox_str;
    }
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

void Core::removeSystemMessage(const long long &chat_id, const std::int32_t &message_id)
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

void Core::addNewMemberChatInDb(TgBot::Message::Ptr &message){

    SQLiteWrapper::Get()->execute("chats.db",
                                  "CREATE TABLE IF NOT EXISTS chat_members (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                  "chat_id INTEGER,"
                                  "user_id INTEGER)");

    if(message->from->id)
    {
        if (message->chat->type == TgBot::Chat::Type::Group ||
            message->chat->type == TgBot::Chat::Type::Supergroup)
        {
            bool find = false;
            auto result_cm = SQLiteWrapper::Get()->retrieve("chats.db", "chat_members:chat_id::int,user_id::int");
            if (result_cm.find("chat_members") != result_cm.end() && !result_cm["chat_members"].empty())
            {
                for (const auto &row : result_cm["chat_members"])
                {
                    if (row.find("chat_id") != row.end() && row.find("user_id") != row.end())
                    {
                        long long userIdDb = std::stoll(row.at("user_id"));
                        long long chatIdDb = std::stoll(row.at("chat_id"));
                        if(userIdDb == message->from->id && chatIdDb == message->chat->id){
                            find = true;
                            break;
                        }
                    }
                }
            }
            if(!find){
                SQLiteWrapper::Get()->add("chats.db", {
                        "chat_members:chat_id:int:" + std::to_string(message->chat->id) +
                        ",chat_members:user_id:int:" + std::to_string(message->from->id)});
            }
        }
    }
}

void Core::getChatMembers(const long long &chat_id, std::vector<std::int32_t> &members)
{
    auto result_cm = SQLiteWrapper::Get()->retrieve("chats.db", "chat_members:chat_id::int,user_id::int");
    if (result_cm.find("chat_members") != result_cm.end() && !result_cm["chat_members"].empty()) {
        for (const auto &row: result_cm["chat_members"]) {
            if (row.find("chat_id") != row.end() && row.find("user_id") != row.end()) {
                long long chatIdDb = std::stoll(row.at("chat_id"));
                if(chatIdDb == chat_id){
                    members.push_back(static_cast<std::int64_t>(std::stoull(row.at("user_id"))));
                }
            }
        }
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

    addNewMemberChatInDb(message);

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
        addNewMemberChatInDb(message);
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

    if (message->text.empty())
        rn;


    const std::string bot_tag = "@" + bot->getApi().getMe()->username + " ";
    Message mess;
    if (message->text.starts_with(bot_tag)) {
        mess.text = message->text;
        mess.text = mess.text.substr(bot_tag.size());
    }
    if(!mess.text.empty()){
        message_history.push_back(mess);
        if (message_history.size() > MAX_HISTORY_MESSAGES_SIZE) {
            message_history.pop_front();
        }

        std::vector<Message> context(message_history.begin(), message_history.end());
        context.pop_back();

        Answer ans = AnswerDatabase::Get()->find_best_match(mess, context);
        bot->getApi().sendMessage(message->chat->id, ans.text);
    }

    bool dbgv = false;
    MemCache::Get()->getKeyValue("botDebug", dbgv);

    std::string chat_type;
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
                bot->getApi().sendMessage(message->chat->id, escapeMarkdownV2("Юзер @" + message->from->username + "( " +
                std::to_string(message->from->id) + " ) ввел команду /" + getCommandName(message->text) +
                " , тип чата: " + chat_type + "."), nullptr, nullptr, nullptr, "MarkdownV2", true);
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
                                      escapeMarkdownV2("Юзер @" + message->from->username + "( " +
                                      std::to_string(message->from->id) + " ) отправил сообщение в чате " +
                                      std::to_string(message->chat->id) + ", тип чата: " + chat_type +
                                      ". Сообщение:\n\n" + message->text), nullptr, nullptr, nullptr, "MarkdownV2", true);
        }
    }
}