#include "Commands/slash_command.h"
#include "dpp/appcommand.h"

dpp::slashcommand Kitty::Commands::SlashCommand::get_dpp_command(dpp::snowflake bot_id) const
{
  dpp::slashcommand cmd(this->name, this->description, bot_id);

  if (this->m_permissions != 0)
  {
    cmd.set_default_permissions(this->m_permissions);
  }
  
  for (const dpp::command_option &opt : this->options())
  {
    cmd.add_option(opt);
  }

  return cmd;
}

std::vector<dpp::command_option> Kitty::Commands::SlashCommand::options() const
{
  return {};
}
