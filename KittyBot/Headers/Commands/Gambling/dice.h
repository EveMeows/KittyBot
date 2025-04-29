#pragma once

#include "Commands/slash_command.h"
#include "Models/user.h"
#include "Services/shared_services.h"
#include "dpp/cluster.h"
#include "dpp/appcommand.h"
#include "dpp/dispatcher.h"
#include <cstdint>
#include <memory>
#include <vector>
namespace Kitty::Commands::Gambling
{
  class Dice : public Commands::SlashCommand
  {
  public:
    Dice(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services)
      : Commands::SlashCommand("die", "Roll the dice, gamble on it's infinite wisdom!")
    {
      this->m_client = client;
      this->m_services = services;
    };

    std::vector<dpp::command_option> options() const override;
    
    void execute(const dpp::slashcommand_t& event) override;
  private:
    bool is_empty(std::vector<dpp::command_data_option> options, const dpp::slashcommand_t& event);
    void update_user(Models::KUser user, uint64_t user_id, uint64_t guild_id);
        
    dpp::cluster* m_client;
    std::shared_ptr<Services::SharedServices> m_services;
  };
}
