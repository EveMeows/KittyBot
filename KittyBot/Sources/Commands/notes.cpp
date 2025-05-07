#include "Commands/notes.h"
#include "appcommand.h"
#include "dispatcher.h"

std::vector<dpp::command_option> Kitty::Commands::Notes::options() const
{
  return {
    dpp::command_option(dpp::command_option_type::co_sub_command, "create", "Create a new note!")
      .add_option(dpp::command_option(dpp::command_option_type::co_string, "name", "Note name."))
      .add_option(dpp::command_option(dpp::command_option_type::co_string, "content", "Note content.")),

    dpp::command_option(dpp::command_option_type::co_sub_command, "use", "Use a note!")
      .add_option(dpp::command_option(dpp::command_option_type::co_string, "name", "Note name."))
  };
}

void Kitty::Commands::Notes::create(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd)
{

}

void Kitty::Commands::Notes::use(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd)
{

}

void Kitty::Commands::Notes::execute(const dpp::slashcommand_t& event)
{

}
