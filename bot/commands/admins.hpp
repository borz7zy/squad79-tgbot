#pragma once

#include "../core.hpp"
#include "command.hpp"
#include "permissions.hpp"
#include "../../utils/escapeMarkdownV2.hpp"

class AdminsCmd : public Command, public Singleton<AdminsCmd>
{
public:
    void execute(TgBot::Message::Ptr &message) override
    {
        try
        {
            std::vector<TgBot::ChatMember::Ptr> admins = Core::Get()->bot->getApi().getChatAdministrators(message->chat->id);
            std::ostringstream ss;

            ss << "📌 Администраторы чата:\n\n";
            for (const auto &admin : admins)
            {
                std::string statusText = PermissionsCmd::Get()->getUserStatusText(admin);
                ss << "👤 " << "[" + escapeMarkdownV2(admin->user->firstName) + "](tg://user?id=" + std::to_string(admin->user->id) + ")" << "\n";
                ss << statusText << "\n";
            }

            Core::Get()->bot->getApi().sendMessage(message->chat->id, ss.str(), nullptr, nullptr, nullptr, "MarkdownV2", true);
        }
        catch (TgBot::TgException &e)
        {
            Logger::Get()->Log("/permissions: Ошибка получения списка администраторов: %s", e.what());
            Core::Get()->bot->getApi().sendMessage(message->chat->id, "Ошибка при получении списка администраторов.");
        }
    }
};
