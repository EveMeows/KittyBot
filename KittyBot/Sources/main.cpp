#include "Commands/Administrative/add_coins.h"
#include "Commands/Administrative/enroll.h"
#include "Commands/Administrative/remove_coins.h"
#include "Commands/Gambling/roulette.h"
#include "Commands/User/uinfo.h"
#include "Commands/kitty.h"
#include "Commands/manager.h"
#include "Commands/ping.h"
#include "Commands/Gambling/dice.h"
#include "Models/user.h"
#include "Services/db.h"
#include "Services/db_init.h"
#include "Services/shared_services.h"
#include "parse_env.h"

#include <cmath>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <format>
#include <functional>

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>
#include <iostream>
#include <memory>

namespace {
  static constexpr bool delete_defaults = false;

  static constexpr float level_increase = 0.15f;
  static constexpr int level_gap = 2;

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

    // User
    manager.enroll<Kitty::Commands::User::UserInfo>();

    // Administrator
    manager.enroll<Kitty::Commands::Administrative::Enroll>();
    // -- Coin commands
    manager.enroll<Kitty::Commands::Administrative::AddCoins>();
    manager.enroll<Kitty::Commands::Administrative::RemoveCoins>();
    // -- XP commands
    // TODO

    // Gambling
    manager.enroll<Kitty::Commands::Gambling::Roulette>();
    manager.enroll<Kitty::Commands::Gambling::Dice>();
    
    // Economy
    // TODO: Dailies and other bs
  }

  static void on_message(const dpp::message_create_t& event, dpp::cluster* client, std::shared_ptr<Kitty::Services::SharedServices> services)
  {
    // Bot safe guard
    if (event.msg.author.is_bot()) return;

    // Check DB
    if (!Kitty::Services::DB::guild_enrolled(services, event.msg.guild_id)) return;

    // If the server is in the database, create or fetch the message owner.
    Kitty::Models::KUser user = Kitty::Services::DB::ensure_user(services, event.msg.author.id, event.msg.guild_id);
    user.xp += user.xpstep;

    // Handle level ups
    if (user.xp >= user.xpnext)
    {
      while (user.xp >= user.xpnext)
      {
        user.xp -= user.xpnext;
        
        user.level += 1;
        if (user.level % 5 == 0) user.xpstep += 5;

        user.xpnext = static_cast<int>(std::floor(std::pow((user.level / level_increase), level_gap)));
      }

      event.reply(std::format("Congrats, {}! You've reached level {}!", event.msg.author.get_mention(), user.level));
    }

    // Update user data
    try
    {
      pqxx::work trans(*services->client);
      trans.exec(R"(
          UPDATE guildmember SET xp = $1, xpstep = $2, xpnext = $3, level = $4, coins = $5
          WHERE memberid = $6 AND guildid = $7;
        )",
        pqxx::params {
          user.xp, user.xpstep, user.xpnext, user.level, user.coins,
          static_cast<uint64_t>(event.msg.author.id),
          static_cast<uint64_t>(event.msg.guild_id)
        }
      );
 
      trans.commit();
    }
    catch (const std::exception& e)
    {
      client->log(dpp::loglevel::ll_error, std::format("Failed to update user message data: {}", e.what()));
      return;
    }
  }

  inline std::string require_env(const char* key) {
    const char* value = std::getenv(key);
    if (!value) {
      std::cerr << "ERROR: Variable " << key << " is not set. Aborting." << std::endl;
      exit(1);
    }
    
    return value;
  }
}

int main()
{
  // Parse the provided .env
  Kitty::parse_env();
  std::string token = require_env("KITTY_TOKEN");

  // Create shared service data
  std::shared_ptr<Kitty::Services::SharedServices> services =
    std::make_shared<Kitty::Services::SharedServices>();

  std::string host = require_env("HOST");
  std::string port = require_env("PORT");
  std::string db   = require_env("DB");
  std::string user = require_env("USER");
  std::string pwd  = require_env("PWD");

  // Attempt database connection.
  try
  {
    services->client = std::make_unique<pqxx::connection>(std::format(
      "host={} port={} dbname={} user={} password={}",
      host, port, db, user, pwd
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
  dpp::cluster client(token, dpp::intents::i_default_intents | dpp::intents::i_message_content);

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

  // On message
  client.on_message_create([&client, &services](const dpp::message_create_t& event) {
    on_message(event, &client, services);
  });

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
