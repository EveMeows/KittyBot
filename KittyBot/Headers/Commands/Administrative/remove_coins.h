#pragma once

#include "Commands/slash_command.h"
#include "Services/db.h"
#include "Services/shared_services.h"
#include <dpp/cluster.h>
#include <dpp/colors.h>
namespace Kitty::Commands::Administrative
{
  class RemoveCoins : public Commands::SlashCommand
  {
  public:
    RemoveCoins(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services)
      : Commands::SlashCommand("removecoins", "Remove coins from a user.")    
    {
      this->m_permissions = dpp::permissions::p_administrator;
      
      this->m_client = client;
      this->m_services = services;
    };

    std::vector<dpp::command_option> options() const override;
    void execute(const dpp::slashcommand_t& event) override;
  private:
    dpp::cluster* m_client;
    std::shared_ptr<Services::SharedServices> m_services;
  };
}
