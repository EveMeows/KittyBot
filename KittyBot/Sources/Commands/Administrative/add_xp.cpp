#include "Commands/Administrative/add_xp.h"
#include "Services/db.h"
#include "dpp/appcommand.h"
#include "dpp/dispatcher.h"
#include "dpp/colors.h"
#include <vector>

std::vector<dpp::command_option> Kitty::Commands::Administrative::AddXP::options() const
{
  return {
    dpp::command_option(dpp::command_option_type::co_integer, "xp", "The amount of XP to add.", true),
    dpp::command_option(dpp::command_option_type::co_user, "user", "The user to add the coins to.", false)
  };
}

void Kitty::Commands::Administrative::AddXP::execute(const dpp::slashcommand_t& event)
{
    
  // Check guild
  if (!Services::DB::guild_enrolled(this->m_services, event.command.guild_id))
  {
    event.reply("Your guild is not yet part of the database! Run /enroll.");
    return;
  }
  
  std::optional<long> xp = this->param<long>("xp", event);
  if (!xp)
  {
    this->m_client->log(dpp::loglevel::ll_error, "How does this even happen?");
    return;
  }

  dpp::user user = event.command.get_issuing_user();
  std::optional<dpp::snowflake> id = this->param<dpp::snowflake>("user", event);
  if (id) user = event.command.get_resolved_user(*id);

  if (*xp <= 0 || user.is_bot())
  {
    event.reply("You can't do that!");
    return;
  }

  // Get user
  Models::KUser db_user = Services::DB::ensure_user(this->m_services, user.id, event.command.guild_id);
  db_user.xp += *xp;

  if (db_user.xp >= db_user.xpnext)
  {
    while (db_user.xp >= db_user.xpnext)
    {
      db_user.xp -= db_user.xpnext;
      
      db_user.level += 1;
      if (db_user.level % 5 == 0) db_user.xpstep += 5;

      db_user.xpnext = static_cast<int>(std::floor(std::pow((db_user.level / this->level_increase), this->level_gap)));
    }
  }

  // Update user
  try
  {
    pqxx::work trans(*this->m_services->client);
    trans.exec(R"(
        UPDATE guildmember SET coins = $1, xp = $2, xpstep = $3, xpnext = $4, level = $5
        WHERE memberid = $6 AND guildid = $7;
      )",
      pqxx::params {
        db_user.coins, db_user.xp, db_user.xpstep, db_user.xpnext, db_user.level,
        static_cast<uint64_t>(user.id),
        static_cast<uint64_t>(event.command.guild_id)
      }
    );

    trans.commit();
  }
  catch (const std::exception& e)
  {
    this->m_client->log(dpp::loglevel::ll_error, std::format("Failed to update user registry: {}", e.what()));
    event.reply("The user XP amount could not be updated...");

    return;
  }

  dpp::embed embed = dpp::embed()
    .set_title("XP transaction results!")
    .set_description(std::format("Successfully transferred {} XP to {}'s bank account!", *xp, user.get_mention()))
    .set_colour(dpp::colours::yellow);
  
  event.reply(embed); 
}
