#pragma once

#include "command.hpp"
#include "../../utils/logger.hpp"

class PermissionsCmd : public Command, public Singleton<PermissionsCmd>
{
public:
    std::string getUserStatusText(const TgBot::ChatMember::Ptr &member)
    {
        if (!member)
            return "Не удалось получить информацию о пользователе.";

        std::ostringstream ss;
        ss << "Статус пользователя в чате: ";

        if (member->status == "creator")
            ss << "👑 Создатель\n";
        else if (member->status == "administrator")
            ss << "🔧 Администратор\n";
        else if (member->status == "member")
            ss << "👤 Обычный участник\n";
        else if (member->status == "restricted")
            ss << "⚠ Ограниченный\n";
        else if (member->status == "left")
            ss << "🚪 Покинул чат\n";
        else if (member->status == "kicked")
            ss << "❌ Забанен\n";
        else
            ss << "❓ Неизвестный статус\n";

        return ss.str();
    }

    void execute(TgBot::Message::Ptr message) override
    {
        if (message->chat->type == TgBot::Chat::Type::Group || message->chat->type == TgBot::Chat::Type::Supergroup)
        {
            try
            {
                TgBot::ChatMember::Ptr chatMember = Core::Get()->bot->getApi().getChatMember(message->chat->id, message->from->id);
                if (!chatMember)
                {
                    Core::Get()->bot->getApi().sendMessage(message->chat->id, "Не удалось получить информацию о правах пользователя.");
                    rn;
                }

                std::string statusText = getUserStatusText(chatMember);
                Core::Get()->bot->getApi().sendMessage(message->chat->id, statusText);
            }
            catch (TgBot::TgException &e)
            {
                Logger::Get()->Log("/permissions: Ошибка получения прав: %s", std::string(e.what()).c_str());
                Core::Get()->bot->getApi().sendMessage(message->chat->id, "Ошибка при получении статуса пользователя.");
            }
        }
    }
};
