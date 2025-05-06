#include "Commands/Administrative/notes_admin.h"
#include "Services/db.h"
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
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

void Kitty::Commands::Administrative::ModNotes::creation(const dpp::slashcommand_t& event, const dpp::command_data_option& subcmd)
{

}

void Kitty::Commands::Administrative::ModNotes::level(const dpp::slashcommand_t& event, const dpp::command_data_option& subcmd)
{

}

void Kitty::Commands::Administrative::ModNotes::prefix(const dpp::slashcommand_t& event, const dpp::command_data_option& subcmd)
{

}

void Kitty::Commands::Administrative::ModNotes::stats(const dpp::slashcommand_t& event, const dpp::command_data_option& subcmd)
{

}

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
