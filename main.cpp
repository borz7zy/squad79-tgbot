#include "main.hpp"
#include "./utils/logger.hpp"
#include "./bot/core.hpp"

#include <csignal>

std::atomic<bool> g_shutdownRequested = false;

void shutdownMain()
{
    if (g_shutdownRequested)
        rn;

    g_shutdownRequested = true;

    Logger::Get()->Log("Shutting down the bot.");

    exit(0);
}

void signalHandler(int signal)
{
    Logger::Get()->Log("Signal received: %d", signal);
    shutdownMain();
}

int main()
{
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    Logger::Get()->Log("Main start");
    TgBot::TgLongPoll longPoll(*Core::Get()->bot);
    while (!g_shutdownRequested) {
        try {
            longPoll.start();
//            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        catch (TgBot::TgException &e) {
            Logger::Get()->Log("Error: %s", e.what());
//            std::this_thread::sleep_for(std::chrono::milliseconds(30000));
        }
    }

    shutdownMain();

    rn 0;
}