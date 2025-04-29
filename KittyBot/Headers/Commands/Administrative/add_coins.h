#pragma once

#include "Commands/slash_command.h"
#include "Models/user.h"
#include "Services/db.h"
#include "Services/shared_services.h"
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/colors.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>
#include <exception>
#include <memory>
#include <vector>
namespace Kitty::Commands::Administrative
{
  class AddCoins : public Commands::SlashCommand
  {
  public:
    AddCoins(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services) : Commands::SlashCommand("addcoins", "Give coins to an user.")
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
