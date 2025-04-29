#include "Commands/Administrative/add_coins.h"
#include "dpp/appcommand.h"
#include <vector>

std::vector<dpp::command_option> Kitty::Commands::Administrative::AddCoins::options() const
{
  return {
    dpp::command_option(dpp::command_option_type::co_integer, "coins", "The amount of coins to add.", true),
    dpp::command_option(dpp::command_option_type::co_user, "user", "The user to add the coins to.", false)
  };
}

void Kitty::Commands::Administrative::AddCoins::execute(const dpp::slashcommand_t& event)
{
  // Check guild
  if (!Services::DB::guild_enrolled(this->m_services, event.command.guild_id))
  {
    event.reply("Your guild is not yet part of the database! Run /enroll.");
    return;
  }
  
  std::optional<long> coins = this->param<long>("coins", event);
  if (!coins)
  {
    this->m_client->log(dpp::loglevel::ll_error, "How does this even happen?");
    return;
  }

  dpp::user user = event.command.get_issuing_user();
  std::optional<dpp::snowflake> id = this->param<dpp::snowflake>("user", event);
  if (id) user = event.command.get_resolved_user(*id);

  
  if (*coins <= 0 || user.is_bot())
  {
    event.reply("You can't do that!");
    return;
  }

  // Get user
  Models::KUser db_user = Services::DB::ensure_user(this->m_services, user.id, event.command.guild_id);
  db_user.coins += *coins;

  // Update user
  try
  {
    pqxx::work trans(*this->m_services->client);
    trans.exec(R"(
        UPDATE guildmember SET coins = $1
        WHERE memberid = $2 AND guildid = $3;
      )",
      pqxx::params {
        db_user.coins,
        static_cast<uint64_t>(user.id),
        static_cast<uint64_t>(event.command.guild_id)
      }
    );

    trans.commit();
  }
  catch (const std::exception& e)
  {
    this->m_client->log(dpp::loglevel::ll_error, std::format("Failed to update user registry: {}", e.what()));
    event.reply("The user coin amount could not be updated...");

    return;
  }

  dpp::embed embed = dpp::embed()
    .set_title("Coins transaction results!")
    .set_description(std::format("Successfully transferred {} coins to {}'s bank account!", *coins, user.get_mention()))
    .set_colour(dpp::colours::yellow);
  
  event.reply(embed); 
}
