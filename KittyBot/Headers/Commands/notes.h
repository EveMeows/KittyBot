#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include "dpp/cluster.h"
#include <memory>
namespace Kitty::Commands
{
  class Notes : public Commands::SlashCommand
  {
  public:
    Notes(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services)
      : Commands::SlashCommand("note", "Handle notes!")
    {
      this->m_client = client;
      this->m_services = services;
    };

    std::vector<dpp::command_option> options() const override;
    void execute(const dpp::slashcommand_t& event) override;
  private:
    dpp::cluster* m_client;
    std::shared_ptr<Services::SharedServices> m_services;

    void create(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd);
    void use(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd);
  };
}
