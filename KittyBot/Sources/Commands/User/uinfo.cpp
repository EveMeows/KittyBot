#include "Commands/User/uinfo.h"
#include "dpp/appcommand.h"
#include "Models/user.h"
#include "Services/db.h"
#include "dpp/message.h"
#include "dpp/snowflake.h"
#include <cstdint>
#include <format>
#include <vector>

std::vector<dpp::command_option> Kitty::Commands::User::UserInfo::options() const
{
  return {
    dpp::command_option(dpp::command_option_type::co_user, "user", "The user to show stats for.", false)
  };
}

void Kitty::Commands::User::UserInfo::execute(const dpp::slashcommand_t& event)
{
  uint64_t guild_id = static_cast<uint64_t>(event.command.guild_id);
  
  // Check DB state
  if (!Services::DB::guild_enrolled(this->m_services, guild_id))
  {
    event.reply("The guild isn't enrolled in the database!");
    return;
  }

  // Handle user input
  dpp::user user = event.command.get_issuing_user();
  std::optional<dpp::snowflake> uid = this->param<dpp::snowflake>("user", event);
  if (uid) user = event.command.get_resolved_user(*uid);
  
  uint64_t user_id = static_cast<uint64_t>(user.id);

  // Create user if not exists
  Models::KUser kuser = Services::DB::ensure_user(this->m_services, user_id, guild_id);

  dpp::user issuing = event.command.get_issuing_user();
  
  int p = std::floor((static_cast<double>(kuser.xp) / kuser.xpnext) * 100);
  std::string stats = std::format(
    "Data statistics for {}.\n{} Coins\nLevel: {}\nXP: {}/{} ({}%)",
    user.get_mention(), kuser.coins,
    kuser.level,
    kuser.xp, kuser.xpnext, p
  );
  
  dpp::embed e = dpp::embed()
    // Top
    .set_title("User statistics!")
    .set_description(stats)

    // Right
    .set_thumbnail(user.get_avatar_url())

    // Bottom
    .set_footer(
      dpp::embed_footer()
        .set_text(std::format("Requested by {}", issuing.username))
        .set_icon(issuing.get_avatar_url())
    )
    .set_timestamp(time(nullptr));

  event.reply(e);
}
