#include "Commands/Administrative/add_coins.h"
#include "Commands/Administrative/add_xp.h"
#include "Commands/Administrative/enroll.h"
#include "Commands/Administrative/notes_admin.h"
#include "Commands/Administrative/remove_coins.h"
#include "Commands/Gambling/roulette.h"
#include "Commands/User/uinfo.h"
#include "Commands/daily.h"
#include "Commands/kitty.h"
#include "Commands/manager.h"
#include "Commands/notes.h"
#include "Commands/ping.h"
#include "Commands/Gambling/dice.h"
#include "Models/guild.h"
#include "Models/user.h"
#include "Services/db.h"
#include "Services/db_init.h"
#include "Services/shared_services.h"
#include "parse_env.h"

#include <cmath>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <format>
#include <functional>

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace {
  static constexpr bool delete_defaults = false;

  static std::function<void()> signal_handle;
  static void exit_handler(int signal)
  {
    if (signal == SIGINT)
    {
      if (signal_handle) signal_handle();
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
    manager.enroll<Kitty::Commands::Administrative::AddXP>();
    // -- Note commands
    manager.enroll<Kitty::Commands::Administrative::ModNotes>();

    // Gambling
    manager.enroll<Kitty::Commands::Gambling::Roulette>();
    manager.enroll<Kitty::Commands::Gambling::Dice>();
    // TODO: Slots

    // Economy
    manager.enroll<Kitty::Commands::Daily>();
    // TODO: Other bs

    // Notes
    manager.enroll<Kitty::Commands::Notes>();
  }

  static std::unordered_map<uint64_t, std::time_t> user_cooldowns;
  static void on_message(const dpp::message_create_t& event, dpp::cluster* client, std::shared_ptr<Kitty::Services::SharedServices> services)
  {
    // Bot safe guard
    if (event.msg.author.is_bot()) return;

    // Check DB
    std::optional<Kitty::Models::KGuild> guild = Kitty::Services::DB::maybe_guild(services, event.msg.guild_id);
    if (!guild) return;

    // #region Note
    std::string msg = event.msg.content;
    if (msg.starts_with(guild->note_prefix))
    {
      // We do not have a split func lmao
      std::string rest = msg.substr(guild->note_prefix.size());
      size_t space = rest.find(' ');
      std::string name = (space == std::string::npos) ? rest : rest.substr(0, space);

      try
      {
        pqxx::work trans(*services->client);
        pqxx::result query = trans.exec(
          "SELECT name, content, guildid FROM note WHERE guildid = $1 AND name = $2",
          pqxx::params { static_cast<uint64_t>(event.msg.guild_id), name }
        );

        if (query.empty()) throw std::runtime_error("No note lmao.");

        pqxx::row note = query.back();
        event.reply(note["content"].as<std::string>());
      }
      catch (const std::exception& e)
      {
        std::cerr << "ERROR: Cannot find note message: " << e.what() << std::endl;
      }
    }
    // #endregion

    // Check if the user is in cooldown. (2s)
    uint64_t uid = event.msg.author.id;
    if (user_cooldowns.find(uid) != user_cooldowns.end())
    {
      // Found user in cooldown map
      std::time_t current = std::time(nullptr);
      std::time_t cooldown = user_cooldowns[uid];

      if (std::difftime(current, cooldown) <= 2) return;
      else user_cooldowns[uid] = current;
    }
    else
    {
      // User not found in cooldown map, add then proceed
      std::time_t current = std::time(nullptr);
      user_cooldowns[uid] = current;
    }

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

        user.xpnext = static_cast<int>(
          std::floor(
            std::pow(
              (user.level / Kitty::Commands::Administrative::AddXP::level_increase), Kitty::Commands::Administrative::AddXP::level_gap)
            )
        );
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

  static std::string require_env(const char* key) {
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
