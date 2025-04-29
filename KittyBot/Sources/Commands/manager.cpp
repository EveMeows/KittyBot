#include "Commands/manager.h"
#include "dpp/appcommand.h"

void Kitty::Commands::CommandManager::create_all()
{
  // Bulk create to save API calls.
  std::vector<dpp::slashcommand> commands;
  dpp::snowflake id = this->m_client->me.id;
  for (const std::unique_ptr<SlashCommand> &command : this->m_commands)
  {
    commands.push_back(command->get_dpp_command(id));
  }

  this->m_client->global_bulk_command_create(commands);
}

void Kitty::Commands::CommandManager::handle(const dpp::slashcommand_t& event)
{
  for (std::unique_ptr<SlashCommand> &command : this->m_commands)
  {
    if (event.command.get_command_name() == command->name)
    {
      command->execute(event);
      return;
    }
  }
}
