#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include <dpp/cluster.h>
namespace Kitty::Commands
{
  class Daily : public Commands::SlashCommand
  {
  public:
    Daily(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services)
      : Kitty::Commands::SlashCommand("daily", "Retrieve your daily reward!")
    {
      this->m_client = client;
      this->m_services = services;
    }

    void execute(const dpp::slashcommand_t &event) override;
  private:
    dpp::cluster* m_client;
    std::shared_ptr<Services::SharedServices> m_services;
  };
}
