#include "Commands/Administrative/notes_admin.h"
#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <vector>

std::vector<dpp::command_option> Kitty::Commands::Administrative::ModNotes::options() const
{
  return {
    dpp::command_option(dpp::command_option_type::co_sub_command, "usercreation", "Enable or disable user ability to create notes.")
      .add_option(dpp::command_option(dpp::command_option_type::co_boolean, "enable", "Wether to enable or disable the creation ability.")),

    dpp::command_option(dpp::command_option_type::co_sub_command, "userlevel", "Set the minimum user level to create notes.")
      .add_option(dpp::command_option(dpp::command_option_type::co_integer, "level", "The minimum level.")),

    dpp::command_option(dpp::command_option_type::co_sub_command, "prefix", "Set the note prefix for messages.")
      .add_option(dpp::command_option(dpp::command_option_type::co_string, "prefix", "The prefix.")),

    dpp::command_option(dpp::command_option_type::co_sub_command, "stats", "Show the notes stats."),
  };
}

void Kitty::Commands::Administrative::ModNotes::execute(const dpp::slashcommand_t& event)
{
  // TODO: Impl
}
