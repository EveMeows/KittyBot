#include "Commands/manager.h"
#include "Commands/ping.h"
#include "parse_env.h"

#include <csignal>
#include <cstdlib>
#include <functional>

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>

// Global handler :/
std::function<void()> signal_handle;
void exit_handler(int signal)
{
    if (signal == SIGINT)
    {
        if (signal_handle) signal_handle();
        exit(0);
    }
}

int main()
{
    // Parse the provided .env
    Kitty::parse_env();
    std::string token = getenv("KITTY_TOKEN");

    // Create client cluster
    dpp::cluster client(token);

    // Register the signal handler.
    signal_handle = [&client]() {
        client.log(dpp::loglevel::ll_info, "Ending Discord session. Goodbye!");
        client.shutdown();
    };

    std::signal(SIGINT, exit_handler);

    // Create logging service
    client.on_log(dpp::utility::cout_logger());

    // Create command manager.
    Kitty::Commands::CommandManager manager(&client);
    manager.enroll<Kitty::Commands::Ping>();

    client.on_slashcommand([&manager](const dpp::slashcommand_t &event) {
        manager.handle(event);
    });

    client.on_ready([&client, &manager](const dpp::ready_t& event)  {
        // Register all commands only once.
        if (dpp::run_once<struct register_bot_commands>()) {
            manager.create_all();
        }

        client.log(dpp::loglevel::ll_info, "BOT: Logged in.");
    });

    // Wait forever.
    client.start(dpp::st_wait);
}
