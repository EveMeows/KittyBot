#include "Commands/Administrative/notes_admin.h"
#include "Services/db.h"
#include "Services/shared_services.h"
#include "message.h"
#include <ctime>
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <exception>
#include <format>
#include <pqxx/internal/statement_parameters.hxx>
#include <stdexcept>
#include <vector>

std::vector<dpp::command_option> Kitty::Commands::Administrative::ModNotes::options() const
{
  return {
    dpp::command_option(dpp::command_option_type::co_sub_command, "usercreation", "Enable or disable user ability to create notes.")
      .add_option(dpp::command_option(dpp::command_option_type::co_boolean, "enable", "Wether to enable or disable the creation ability.", true)),

    dpp::command_option(dpp::command_option_type::co_sub_command, "userlevel", "Set the minimum user level to create notes.")
      .add_option(dpp::command_option(dpp::command_option_type::co_integer, "level", "The minimum level.", true)),

    dpp::command_option(dpp::command_option_type::co_sub_command, "prefix", "Set the note prefix for messages.")
      .add_option(dpp::command_option(dpp::command_option_type::co_string, "prefix", "The prefix.", true)),

    dpp::command_option(dpp::command_option_type::co_sub_command, "stats", "Show the notes stats."),
  };
}

// #region
void Kitty::Commands::Administrative::ModNotes::creation(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd)
{
  try
  {
    pqxx::work trans(*this->m_services->client);

    uint64_t gid = static_cast<uint64_t>(event.command.guild_id);
    bool allow = subcmd.get_value<bool>(0);

    trans.exec(R"(
        UPDATE guild SET noteallow = $1 WHERE id = $2
      )",
      pqxx::params {
        allow, gid
      }
    );

    trans.commit();

    event.reply("Updated note creation state!");
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not update data: " << e.what() << std::endl;
  }
}

void Kitty::Commands::Administrative::ModNotes::level(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd)
{
  try
  {
    pqxx::work trans(*this->m_services->client);

    uint64_t gid = static_cast<uint64_t>(event.command.guild_id);
    long int lvl = subcmd.get_value<long int>(0);

    trans.exec(R"(
        UPDATE guild SET noteminlevel = $1 WHERE id = $2
      )",
      pqxx::params {
        lvl, gid
      }
    );

    trans.commit();

    event.reply("Updated note level!");
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not update data: " << e.what() << std::endl;
  }
}

void Kitty::Commands::Administrative::ModNotes::prefix(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd)
{
  try
  {
    pqxx::work trans(*this->m_services->client);

    uint64_t gid = static_cast<uint64_t>(event.command.guild_id);
    std::string prefix = subcmd.get_value<std::string>(0);

    trans.exec(R"(
        UPDATE guild SET noteprefix = $1 WHERE id = $2
      )",
      pqxx::params {
        prefix, gid
      }
    );

    trans.commit();

    event.reply("Updated note prefix!");
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not update data: " << e.what() << std::endl;
  }
}

void Kitty::Commands::Administrative::ModNotes::stats(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd)
{
  try
  {
    pqxx::work trans(*this->m_services->client);
    pqxx::result query = trans.exec(R"(
        SELECT id, noteprefix, noteminlevel, noteallow FROM guild
        WHERE id = $1
      )",
      pqxx::params {
        static_cast<uint64_t>(event.command.guild_id)
      }
    );

    if (query.empty()) throw new std::runtime_error("Query returned nothing.");

    pqxx::row result = query.back();
    std::string description = std::format(
      "Prefix: {}\nMinimum Level: {}\nUser Creation: {}",
      result["noteprefix"].as<std::string>(), result["noteminlevel"].as<int>(),
      result["noteallow"].as<bool>()
    );

    dpp::user issuing = event.command.get_issuing_user();
    dpp::embed embed = dpp::embed()
      .set_title("Note status for this server!")
      .set_description(description)
      .set_footer(
        dpp::embed_footer()
          .set_text(std::format("Requested by: {}", issuing.format_username()))
          .set_icon(issuing.get_avatar_url())
      )
      .set_timestamp(std::time(nullptr));

    event.reply(embed);
  }
  catch (const std::exception& e)
  {
    std::cerr << "ERROR: Could not update data: " << e.what() << std::endl;
  }
}
// #endregion

void Kitty::Commands::Administrative::ModNotes::execute(const dpp::slashcommand_t& event)
{
  if(!Services::DB::guild_enrolled(this->m_services, event.command.guild_id))
  {
    event.reply("The guild isn't part of the database!");
    return;
  }

  dpp::command_interaction cmd = event.command.get_command_interaction();
  dpp::command_data_option subcmd = cmd.options[0];

  // wtf
  if (subcmd.name == "usercreation")   this->creation(event, subcmd);
  else if (subcmd.name == "userlevel") this->level(event, subcmd);
  else if (subcmd.name == "prefix")    this->prefix(event, subcmd);
  else if (subcmd.name == "stats")     this->stats(event, subcmd);
}
