#include "Commands/daily.h"
#include "Models/user.h"
#include "Services/db.h"
#include "Services/shared_services.h"
#include <cstdint>
#include <ctime>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>
#include <exception>
#include <iostream>
#include <pqxx/internal/statement_parameters.hxx>

void Kitty::Commands::Daily::execute(const dpp::slashcommand_t& event)
{
  uint64_t gid = event.command.guild_id;

  dpp::user user = event.command.get_issuing_user();
  uint64_t uid = user.id;

  if (!Services::DB::guild_enrolled(this->m_services, gid))
  {
    event.reply("The guild is not enrolled in the database!");
    return;
  }

  try
  {
    Kitty::Models::KUser kuser = Services::DB::ensure_user(this->m_services, uid, gid);

    pqxx::work trans = pqxx::work(*this->m_services->client);
    pqxx::result res = trans.exec(
      "SELECT lastclaimed, memberid, guildid FROM guildmember "
      "WHERE lastclaimed <= NOW() - INTERVAL '1 day' AND memberid = $1 AND guildid = $2;",
      pqxx::params {uid, gid}
    );

    if (res.empty()) {
      pqxx::result time_check = trans.exec(
        "SELECT lastclaimed FROM guildmember WHERE memberid = $1 AND guildid = $2;",
        pqxx::params {uid, gid}
      );

      std::string lastclaimed_str = time_check[0]["lastclaimed"].c_str();

      std::tm last_tm = {};
      std::istringstream ss(lastclaimed_str);
      ss >> std::get_time(&last_tm, "%Y-%m-%d %H:%M:%S");

      std::time_t last_time = std::mktime(&last_tm);
      std::time_t now = std::time(nullptr);
      std::time_t next_claim_time = last_time + 86400;
      std::time_t diff = next_claim_time - now;

      int hours = diff / 3600;
      int minutes = (diff % 3600) / 60;
      int seconds = diff % 60;

      std::ostringstream remaining;
      if (diff > 0) {
        remaining << "Come back in ";
        if (hours > 0) remaining << hours << "h ";
        if (minutes > 0 || hours > 0) remaining << minutes << "m ";
        remaining << seconds << "s.";
      } else {
        remaining << "Try again now!";
      }

      dpp::embed cannot = dpp::embed()
        .set_title("Hold up!")
        .set_description("You've already claimed your prize for the day!\n" + remaining.str())
        .set_footer(dpp::embed_footer()
          .set_text(user.username)
          .set_icon(user.get_avatar_url()))
        .set_timestamp(time(nullptr));

      event.reply(cannot);
    } else {
      kuser.coins += 1000;

      std::time_t timestamp = time(nullptr);
      std::tm *now = std::localtime(&timestamp);

      std::ostringstream tm_fmt;
      tm_fmt << std::put_time(now, "%Y-%m-%d %H:%M:%S");
      kuser.lastclaimed = tm_fmt.str();

      trans.exec(
        "UPDATE guildmember SET lastclaimed = $1, coins = $2 WHERE memberid = $3 AND guildid = $4;",
        pqxx::params {kuser.lastclaimed, kuser.coins, uid, gid}
      );

      dpp::embed claimed = dpp::embed()
        .set_title("Daily prize!")
        .set_description("You successfully claimed your daily 1000 coins!")
        .set_footer(dpp::embed_footer()
          .set_text(user.username)
          .set_icon(user.get_avatar_url()))
        .set_timestamp(time(nullptr));

      event.reply(claimed);
    }

    trans.commit();
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not process dailies: " << e.what() << "\n";
  }
}
