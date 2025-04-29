#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/message.h>
#include <memory>
#include <mutex>
#include <vector>
namespace Kitty::Commands::Gambling
{
  class Roulette : public Commands::SlashCommand
  {
  public:
    Roulette(dpp::cluster *client, std::shared_ptr<Services::SharedServices> services)
      : Commands::SlashCommand("roulette", "Double your coin... But at what cost?")
    {
      this->m_client = client;
      this->m_services = services;
    }

    std::vector<dpp::command_option> options() const override;
    void execute(const dpp::slashcommand_t &event) override;
    
  private:
    std::mutex m_wager_guard;

    dpp::cluster *m_client;
    std::shared_ptr<Services::SharedServices> m_services;
  };
} // namespace Kitty::Commands::Gambling
