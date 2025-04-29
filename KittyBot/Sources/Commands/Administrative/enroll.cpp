#include "Commands/Administrative/enroll.h"
#include <exception>
#include <format>

void Kitty::Commands::Administrative::Enroll::execute(const dpp::slashcommand_t& event)
{
  try
  {
    pqxx::work insert(*this->m_shared->client);

    // Check if the guild is already in the database
    auto result = insert.exec("SELECT id FROM guild WHERE id = $1;", pqxx::params { static_cast<int64_t>(event.command.guild_id) });
    if (!result.empty())
    {
      event.reply("Your server is already enrolled into my database!");
      return;
    }

    // Insert the guild into the database.
    insert.exec("INSERT INTO guild (id) VALUES ($1) ON CONFLICT DO NOTHING;", pqxx::params { static_cast<int64_t>(event.command.guild_id) });

    insert.commit();
  }
  catch (const std::exception& e)
  {
    this->m_client->log(dpp::loglevel::ll_error, std::format("Could not enroll server inside db: {}", e.what()));
    event.reply("Your server could not be enrolled into the database...");

    return;
  }

  dpp::embed thanks = dpp::embed()
    .set_colour(dpp::colours::pink)
    .set_title("Thank you!")
    .set_description("Your server is now enrolled into my database!\nYour users can now level up and access the economy!");

  event.reply(thanks);
}
