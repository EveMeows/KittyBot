#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <memory>
#include <optional>
#include <vector>
namespace Kitty::Commands::Gambling
{
  class Roulette : public Commands::SlashCommand
  {
  public:
    Roulette(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services) : Commands::SlashCommand("roulette", "Double your coin... But at what cost?")
    {
      this->m_client = client;
    }

    std::vector<dpp::command_option> options() const override
    {
      return {
        dpp::command_option(dpp::command_option_type::co_integer, "wager", "The amount of coins to bet", true),
        dpp::command_option(dpp::command_option_type::co_string, "colour", "The roulette colour to bet on.", true)
          .add_choice(dpp::command_option_choice("Red", std::string("roulette_red")))
          .add_choice(dpp::command_option_choice("Black", std::string("roulette_black")))
      };
    }

    void execute(const dpp::slashcommand_t& event) override
    {
      std::optional<std::string> colour = this->param<std::string>("colour", event);
      if (!colour)
      {
        return;
      }
      
      event.reply("You chose " + *colour);
    }
  private:
    dpp::cluster* m_client;
  };
}
