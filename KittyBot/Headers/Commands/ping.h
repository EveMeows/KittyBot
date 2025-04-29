#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/event.h>
#include <dpp/misc-enum.h>

namespace Kitty::Commands
{
  class Ping : public SlashCommand
  {
  private:
    dpp::cluster *m_client;

  public:
    Ping(dpp::cluster *client, std::shared_ptr<Services::SharedServices> services) : SlashCommand("ping", "Pong!")
    {
      this->m_client = client;
    };

    std::vector<dpp::command_option> options() const override;
    void execute(const dpp::slashcommand_t &event) override;
  };
} // namespace Kitty::Commands
