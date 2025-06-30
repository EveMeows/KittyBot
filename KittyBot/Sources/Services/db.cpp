#include "Services/db.h"
#include "Models/guild.h"
#include "Models/user.h"
#include "Services/shared_services.h"
#include <dpp/snowflake.h>
#include <exception>
#include <iostream>
#include <optional>
#include <pqxx/internal/statement_parameters.hxx>
#include <stdexcept>
#include <string>

Kitty::Models::KUser Kitty::Services::DB::ensure_user(std::shared_ptr<Services::SharedServices> services, dpp::snowflake member_id, dpp::snowflake guild_id)
{
  Kitty::Models::KUser user;

  try
  {
    pqxx::work trans(*services->client);

    pqxx::result res = trans.exec(R"(
        SELECT
          gm.memberid, gm.coins, gm.xp, gm.xpstep, gm.xpnext, gm.level, gm.lastclaimed
        FROM guildmember gm
        WHERE gm.memberid = $1 AND gm.guildid = $2;
      )",

      pqxx::params {
        static_cast<uint64_t>(member_id),
        static_cast<uint64_t>(guild_id)
      }
    );

    if (!res.empty())
    {
      for (const pqxx::row row : res)
      {
        user.level = row["level"].as<int>();
        user.coins = row["coins"].as<int>();

        user.xp = row["xp"].as<int>();
        user.xpstep = row["xpstep"].as<int>();
        user.xpnext = row["xpnext"].as<int>();

        user.lastclaimed = row["lastclaimed"].as<std::string>();

        return user;
      }
    }

    // The user does not exist so we add it to the database.
    trans.exec("INSERT INTO member (id) VALUES ($1) ON CONFLICT DO NOTHING;", pqxx::params { static_cast<uint64_t>(member_id) });
    trans.exec("INSERT INTO guildmember (memberid, guildid) VALUES ($1, $2) ON CONFLICT DO NOTHING;", pqxx::params { static_cast<uint64_t>(member_id), static_cast<uint64_t>(guild_id) });

    user.level = 1;
    user.coins = 0;
    user.xp = 0;
    user.xpstep = 1;
    user.xpnext = 30;
    user.lastclaimed = "1970-01-01 02:00:00";

    std::cout << "Enrolled user " << static_cast<uint64_t>(member_id) << " into the database." << std::endl;

    trans.commit();
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not fetch database user: " << e.what() << std::endl;
  }

  return user;
}

bool Kitty::Services::DB::guild_enrolled(std::shared_ptr<Services::SharedServices> services, dpp::snowflake guild_id)
{
  try
  {
    pqxx::work trans(*services->client);

    pqxx::result res = trans.exec("SELECT id FROM guild WHERE id = $1;", pqxx::params { static_cast<uint64_t>(guild_id) });
    if (!res.empty())
    {
      return true;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not fetch database guild: " << e.what() << std::endl;
  }

  return false;
}

std::optional<Kitty::Models::KGuild> Kitty::Services::DB::maybe_guild(std::shared_ptr<Services::SharedServices> services, dpp::snowflake guild_id)
{
  try
  {
    pqxx::work trans(*services->client);

    pqxx::result query = trans.exec(R"(
        SELECT id, noteprefix, noteminlevel, noteallow
        FROM guild WHERE id = $1;
      )",
      pqxx::params { static_cast<uint64_t>(guild_id) }
    );

    if (query.empty()) throw std::runtime_error("Query returned nothing.");

    pqxx::row guild = query.back();

    return Models::KGuild {
      guild["noteprefix"].as<std::string>(),
      guild["noteminlevel"].as<int>(),
      guild["noteallow"].as<bool>()
    };
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not fetch database guild: " << e.what() << std::endl;
  }

  return std::nullopt;
}
