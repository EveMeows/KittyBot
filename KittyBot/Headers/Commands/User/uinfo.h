#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include "dpp/cluster.h"
#include <memory>
#include <vector>

namespace Kitty::Commands::User
{
  class UserInfo : public SlashCommand
  {
  public:
    UserInfo(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services)
      : SlashCommand("stats", "Show user information.")
    {
      this->m_client = client;
      this->m_services = services;
    }

    std::vector<dpp::command_option> options() const override;
    void execute(const dpp::slashcommand_t& event) override;
  private:
    dpp::cluster* m_client;
    std::shared_ptr<Services::SharedServices> m_services;
  };
}
