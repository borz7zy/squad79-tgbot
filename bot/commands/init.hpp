#pragma once

#include "../core.hpp"
#include "command.hpp"
#include "../../database/sqlite_wrapper.hpp"
#include <memory>

class InitCmd : public Command, public Singleton<InitCmd>
{
public:
    void execute(TgBot::Message::Ptr message) override
    {
        if (!message || !message->chat)
        {
            Logger::Get()->Log("/init: Получено некорректное сообщение");
            rn;
        }

        if (message->chat->type == TgBot::Chat::Type::Group || message->chat->type == TgBot::Chat::Type::Supergroup)
        {
            long long chat_id = message->chat->id;
            long long creator_id = 0;

            try
            {
                std::vector<TgBot::ChatMember::Ptr> admins = Core::Get()->bot->getApi().getChatAdministrators(chat_id);
                for (const auto &admin : admins)
                {
                    if (admin && admin->user && admin->status == "creator")
                    {
                        creator_id = admin->user->id;
                        break;
                    }
                }
            }
            catch (const TgBot::TgException &e)
            {
                Logger::Get()->Log("/init: Ошибка при получении администраторов: %s", e.what());
                Core::Get()->bot->getApi().sendMessage(chat_id, "Не удалось получить информацию об администраторах чата.");
                rn;
            }

            if (creator_id == 0)
            {
                Core::Get()->bot->getApi().sendMessage(chat_id, "Не удалось найти создателя чата.");
                rn;
            }

            try
            {
                auto db = SQLiteWrapper::Get();
                if (!db)
                {
                    throw std::runtime_error("Не удалось получить экземпляр SQLiteWrapper");
                }

                // Создаем таблицу, если она не существует, и обновляем схему
                db->execute("main_init.db",
                            "CREATE TABLE IF NOT EXISTS mainchat (id INTEGER PRIMARY KEY AUTOINCREMENT, chat_id INTEGER, creator_id INTEGER)");

#ifdef DEBUG
                Logger::Get()->Log("/init: Таблица mainchat обновлена");
#endif

                // Проверяем структуру таблицы
                auto tableInfo = db->getTableInfo("main_init.db", "mainchat");
#ifdef DEBUG
                Logger::Get()->Log("/init: Структура таблицы mainchat:");
                for (const auto &column : tableInfo)
                {
                    Logger::Get()->Log("  Колонка: %s, Тип: %s", column.first.c_str(), column.second.c_str());
                }
#endif

                // Пробуем получить данные
                auto result = db->retrieve("main_init.db", "mainchat:chat_id::int,creator_id::int");

#ifdef DEBUG
                Logger::Get()->Log("/init: Запрос выполнен успешно");
#endif

                bool inited = false;
                if (result.find("mainchat") != result.end() && !result["mainchat"].empty())
                {
                    for (const auto &row : result["mainchat"])
                    {
#ifdef DEBUG
                        Logger::Get()->Log("/init: Обработка строки из результата запроса");
#endif
                        if (row.find("chat_id") != row.end())
                        {
                            long long chat_id_db = std::stoll(row.at("chat_id"));
                            if (chat_id_db == chat_id)
                            {
                                Core::Get()->bot->getApi().sendMessage(message->chat->id, "Бот уже был инициализирован здесь =)");
                                inited = true;
                            }
                            else
                            {
                                Core::Get()->bot->getApi().sendMessage(message->chat->id, "Бот уже был инициализирован в другом чате!\nБот может быть привязан только к одному чату =(");
                                inited = true;
                            }
                            break;
                        }
                    }
                }
#ifdef DEBUG
                else
                {
                    Logger::Get()->Log("/init: Таблица mainchat пуста или не существует");
                }
#endif

                if (!inited)
                {
#ifdef DEBUG
                    Logger::Get()->Log("/init: Инициализация нового чата");
#endif
                    db->add("main_init.db", {"mainchat:chat_id:int:" + std::to_string(chat_id),
                                             "mainchat:creator_id:int:" + std::to_string(creator_id)});
                    Core::Get()->bot->getApi().sendMessage(chat_id, "Чат был инициализирован!");
                }
            }
            catch (const std::exception &e)
            {
                Logger::Get()->Log("/init: Ошибка: %s", e.what());
                Core::Get()->bot->getApi().sendMessage(chat_id, "Произошла ошибка при инициализации. Пожалуйста, попробуйте позже.");
            }
        }
        else
        {
            Core::Get()->bot->getApi().sendMessage(message->chat->id, "Эта команда может быть использована только в группах или супергруппах.");
        }
    }
};