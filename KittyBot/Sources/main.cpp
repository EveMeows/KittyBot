#include "Commands/Administrative/enroll.h"
#include "Commands/kitty.h"
#include "Commands/manager.h"
#include "Commands/ping.h"
#include "Services/db_init.h"
#include "Services/shared_services.h"
#include "parse_env.h"

#include <csignal>
#include <cstdlib>
#include <exception>
#include <format>
#include <functional>

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>
#include <memory>

namespace {
  static constexpr bool delete_defaults = true;

  // Global handler :/
  static std::function<void()> signal_handle;
  static void exit_handler(int signal)
  {
    if (signal == SIGINT)
    {
      if (signal_handle)
        signal_handle();
      exit(0);
    }
  }

  static void register_commands(Kitty::Commands::CommandManager& manager)
  {
    // Misc
    manager.enroll<Kitty::Commands::Ping>();
    manager.enroll<Kitty::Commands::Kitty>();

    // Administrator
    manager.enroll<Kitty::Commands::Administrative::Enroll>();
  }
}

int main()
{
  // Parse the provided .env
  Kitty::parse_env();
  std::string token = getenv("KITTY_TOKEN");

  // Create shared service data
  std::shared_ptr<Kitty::Services::SharedServices> services =
    std::make_shared<Kitty::Services::SharedServices>();

  // Attempt database connection.
  try
  {
    services->client = std::make_unique<pqxx::connection>(std::format(
      "host={} port={} dbname={} user={} password={}",
      std::getenv("HOST"), std::getenv("PORT"), std::getenv("DB"), std::getenv("USER"), std::getenv("PWD")
    ));

    std::cout << "INFO: Successfully established connection to " << services->client->dbname() << " as " << services->client->username() << "!" << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Connecting to database: " << e.what() << std::endl;
    return -1;
  }

  if (delete_defaults)
  {
    if (!Kitty::Services::DB::Init::drop_base_tables(services)) return -1;
    std::cout << "INFO: Dropped default tables." << std::endl;
  }

  if (!Kitty::Services::DB::Init::create_base_tables(services)) return -1;  
  std::cout << "INFO: Created default tables." << std::endl;

  // Create client cluster
  dpp::cluster client(token);

  // Register the signal handler.
  signal_handle = [&client, &services]() {
    client.log(dpp::loglevel::ll_info, "Ending Discord session. Goodbye!");
    client.shutdown();

    std::cout << "INFO: Severing database connection." << std::endl;
    services->client->close();
  };

  std::signal(SIGINT, exit_handler);

  // Create logging service
  client.on_log(dpp::utility::cout_logger());

  // Create command manager.
  Kitty::Commands::CommandManager manager(&client, services);
  register_commands(manager);
  
  client.on_slashcommand(
    [&manager](const dpp::slashcommand_t &event) {
      manager.handle(event);
    }
  );

  client.on_ready(
    [&client, &manager](const dpp::ready_t &event) {
      // Register all commands only once.
      if (dpp::run_once<struct register_bot_commands>())
      {
        manager.create_all();
      }

      client.log(dpp::loglevel::ll_info, "BOT: Logged in.");
    }
  );

  // Wait forever.
  client.start(dpp::st_wait);
}
