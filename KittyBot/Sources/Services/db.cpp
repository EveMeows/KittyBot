#include "Services/db.h"
#include "Models/user.h"
#include "Services/shared_services.h"
#include <dpp/snowflake.h>
#include <exception>
#include <iostream>
#include <pqxx/internal/statement_parameters.hxx>

Kitty::Models::KUser Kitty::Services::DB::ensure_user(std::shared_ptr<Services::SharedServices> services, dpp::snowflake member_id, dpp::snowflake guild_id)
{
  Kitty::Models::KUser user;
  
  try
  {
    pqxx::work trans(*services->client);

    pqxx::result res = trans.exec_params(R"(
        SELECT
          m.id, gm.coins, gm.xp, gm.xpstep, gm.xpnext, gm.level
        FROM guildmember gm
        JOIN member m ON m.id = gm.memberid
        JOIN guild g ON g.id = gm.guildid
        WHERE m.id = $1 AND gm.guildid = $2;
      )",
      static_cast<uint64_t>(member_id),
      static_cast<uint64_t>(guild_id)
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
        
        return user;
      }
    }

    // The user does not exist so we add it to the database.
    trans.exec_params("INSERT INTO member (id) VALUES ($1) ON CONFLICT DO NOTHING;", static_cast<uint64_t>(member_id));
    trans.exec_params("INSERT INTO guildmember (memberid, guildid) VALUES ($1, $2) ON CONFLICT DO NOTHING;", static_cast<uint64_t>(member_id), static_cast<uint64_t>(guild_id));

    user.level = 1;
    user.coins = 0;
    user.xp = 0;
    user.xpstep = 1;
    user.xpnext = 30;

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

    pqxx::result res = trans.exec_params("SELECT id FROM guild WHERE id = $1;", static_cast<uint64_t>(guild_id));
    if (!res.empty())
    {
      return true;
    }

    trans.commit();
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not fetch database guild: " << e.what() << std::endl;
    return false;
  }
  
  return false;
}
