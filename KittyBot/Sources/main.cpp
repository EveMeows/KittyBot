#include "parse_env.h"

#include <cstdlib>

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>

int main()
{
    // Parse the provided .env
    Kitty::parse_env();
    std::string token = getenv("KITTY_TOKEN");

    // Create client cluster
    dpp::cluster client(token);

    // Create logging service
    client.on_log(dpp::utility::cout_logger());

    client.on_ready([&client](const dpp::ready_t& event)  {
        client.log(dpp::loglevel::ll_info, "BOT: Logged in.");
    });

    // Wait forever.
    client.start(dpp::st_wait);
}
