#include "Commands/notes.h"
#include "Models/guild.h"
#include "Models/user.h"
#include "Services/db.h"
#include "Services/shared_services.h"
#include "appcommand.h"
#include "dispatcher.h"
#include "snowflake.h"
#include <cstdint>
#include <exception>
#include <optional>

std::vector<dpp::command_option> Kitty::Commands::Notes::options() const
{
  return {
    dpp::command_option(dpp::command_option_type::co_sub_command, "create", "Create a new note!")
      .add_option(dpp::command_option(dpp::command_option_type::co_string, "name", "Note name.", true))
      .add_option(dpp::command_option(dpp::command_option_type::co_string, "content", "Note content.", true)),

    dpp::command_option(dpp::command_option_type::co_sub_command, "use", "Use a note!")
      .add_option(dpp::command_option(dpp::command_option_type::co_string, "name", "Note name.", true))
  };
}

void Kitty::Commands::Notes::create(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd)
{
  uint64_t gid = static_cast<uint64_t>(event.command.guild_id);
  uint64_t mid = static_cast<uint64_t>(event.command.get_issuing_user().id);
  std::optional<Models::KGuild> guild = Services::DB::maybe_guild(this->m_services, gid);
  if (!guild)
  {
    event.reply("The guild isn't part of the database!");
    return;
  }

  if (!(*guild).note_allow)
  {
    event.reply("This guild does not allow regular users to create notes.");
    return;
  }

  Models::KUser user = Services::DB::ensure_user(this->m_services, mid, gid);
  if (user.level < (*guild).note_minlevel)
  {
    event.reply("Your level is too low to create a note!");
    return;
  }

  try
  {
    std::string name = subcmd.get_value<std::string>(0);
    std::string content = subcmd.get_value<std::string>(1);

    pqxx::work trans(*this->m_services->client);
    trans.exec(R"(
        INSERT INTO note (name, content, guildid) VALUES ($1, $2, $3);
      )",
      pqxx::params {
        name, content, gid
      }
    );

    trans.commit();

    event.reply(std::format("Successfully created note {}!", name));
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Unable to insert note data: " << e.what() << std::endl;
  }
}

void Kitty::Commands::Notes::use(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd)
{
  uint64_t gid = static_cast<uint64_t>(event.command.guild_id);
  if (!Services::DB::guild_enrolled(this->m_services, gid))
  {
    event.reply("The guild isn't part of the database!");
    return;
  }

  try
  {
    std::string name = subcmd.get_value<std::string>(0);

    pqxx::work trans(*this->m_services->client);
    pqxx::result query = trans.exec("SELECT name, content, guildid FROM note WHERE guildid = $1 AND name = $2", pqxx::params { gid, name });
    if (query.empty())
    {
      event.reply(std::format("No note by the name \"{}\" exists!", name));
      return;
    }

    pqxx::row note = query.back();
    event.reply(note["content"].as<std::string>());
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Cannot retrieve note: " << e.what() << std::endl;
  }
}

void Kitty::Commands::Notes::execute(const dpp::slashcommand_t& event)
{
  dpp::command_interaction cmd = event.command.get_command_interaction();
  dpp::command_data_option subcmd = cmd.options[0];

  if (subcmd.name == "create") this->create(event, subcmd);
  else if (subcmd.name == "use") this->use(event, subcmd);
}
