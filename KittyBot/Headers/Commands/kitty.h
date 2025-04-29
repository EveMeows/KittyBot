#pragma once

#include "Commands/slash_command.h"
#include "Services/api_service.h"
#include "Services/nekosia.h"
#include "Services/shared_services.h"

#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <memory>
#include <ctime>

namespace Kitty::Commands
{
  class Kitty : public SlashCommand
  {
  private:
    dpp::cluster *m_client;
    std::unique_ptr<Services::ApiService> m_api_service;

  public:
    Kitty(dpp::cluster *client, std::shared_ptr<Services::SharedServices> services)
      : SlashCommand("kitty", "Fetch a silly little picture!")
    {
      this->m_client = client;
      this->m_api_service = std::make_unique<Services::Nekosia>();
    }

    void execute(const dpp::slashcommand_t &event) override;
  };
} // namespace Kitty::Commands
