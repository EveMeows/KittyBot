#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <memory>
namespace Kitty::Commands::Administrative
{
  class Enroll : public Commands::SlashCommand
  {
  public:
    Enroll(dpp::cluster* client, std::shared_ptr<Services::SharedServices> shared)
      : Commands::SlashCommand("enroll", "Enroll the server into the database.")
    {
      this->m_shared = shared;
      this->m_client = client;

      this->m_permissions = dpp::permissions::p_administrator;
    }

    void execute(const dpp::slashcommand_t& event) override;
  private:
    std::shared_ptr<Services::SharedServices> m_shared;
    dpp::cluster* m_client = nullptr;
  };
}
