#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include "dpp/appcommand.h"
#include "dpp/cluster.h"
#include "dpp/dispatcher.h"
#include <memory>
#include <vector>
namespace Kitty::Commands::Administrative
{
  class AddXP : public Commands::SlashCommand
  {
  public:
    static constexpr float level_increase = 0.15f;
    static constexpr int level_gap = 2;
    
    AddXP(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services)
      : Commands::SlashCommand("addxp", "Add XP to a user's account!")
    {
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
