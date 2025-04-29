#pragma once

#include "Commands/slash_command.h"
#include "Models/user.h"
#include "Services/db.h"
#include "Services/shared_services.h"
#include "cluster.h"
#include "dpp/appcommand.h"
#include "dpp/appcommand.h"
#include "dpp/colors.h"
#include "dpp/dispatcher.h"
#include "dpp/message.h"
#include <memory>
#include <random>
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

    std::vector<dpp::command_option> options() const override
    {
      return {
        dpp::command_option(dpp::command_option_type::co_sub_command, "exact", "Gamble on the exact answer! Reward is 1:3")
          .add_option(dpp::command_option(dpp::command_option_type::co_integer, "wager", "The amount of coins to bet", true))
          .add_option(dpp::command_option(dpp::command_option_type::co_integer, "number", "The number to gamble for", true)),
          
        dpp::command_option(dpp::command_option_type::co_sub_command, "evenodd", "Gamble on the type of number returned! Reward is 1:2")
          .add_option(dpp::command_option(dpp::command_option_type::co_integer, "wager", "The amount of coins to bet", true))
          .add_option(
            dpp::command_option(dpp::command_option_type::co_string, "type", "Even or Odd", true)
              .add_choice(dpp::command_option_choice("Even", std::string("even")))
              .add_choice(dpp::command_option_choice("Odd", std::string("odd")))
          ),
                    
        dpp::command_option(dpp::command_option_type::co_sub_command, "range", "Gamble on the range of number returned! Reward is 1:2")
          .add_option(dpp::command_option(dpp::command_option_type::co_integer, "wager", "The amount of coins to bet", true))
          .add_option(
            dpp::command_option(dpp::command_option_type::co_string, "range", "Choose a range", true)
              .add_choice(dpp::command_option_choice("1-3", std::string("13")))
              .add_choice(dpp::command_option_choice("4-6", std::string("46")))
          ),
      };
    }

    void execute(const dpp::slashcommand_t& event) override
    {
      if(!Services::DB::guild_enrolled(this->m_services, event.command.guild_id))
      {
        event.reply("The guild isn't part of the database!");
        return;
      }
      
      dpp::command_interaction cmd = event.command.get_command_interaction();
      dpp::command_data_option subcmd = cmd.options[0];

      dpp::snowflake user_id = event.command.get_issuing_user().id;
      Models::KUser user = Services::DB::ensure_user(this->m_services, user_id, event.command.guild_id);

      if (subcmd.name == "exact")
      {
        if (this->is_empty(subcmd.options, event)) return;

        long int wager = subcmd.get_value<long int>(0);
        if (wager < 1 || wager > user.coins)
        {
          event.reply("You cannot bet that amount!");
          return;
        }
        
        long int number = subcmd.get_value<long int>(1);
        if (number < 1 || number > 6)
        {
          event.reply("You cannot bet that number... The die only has 6 sides.");
          return;
        }
        
        event.reply(std::format("Wager: {}\nBet: Exact Roll\nNumber: {}\nTotal Coins: {}\n\n:game_die: Rolling the die... Please hang on!", wager, number, user.coins));

        std::thread([event, wager, user, number, this, user_id]() mutable {
          std::random_device rand;
          std::mt19937 rng(rand());
          std::uniform_int_distribution<int> distr(0, 6);
          
          int side = distr(rng);

          using namespace std::chrono_literals;
          std::this_thread::sleep_for(2s);

          bool won = true;
          if (number != side)
          {
            won = false;
            user.coins -= wager;
          }
          else
          {
            user.coins += wager * 2;
          }

          std::string msg = won ? "You did it! Your wager has been doubled and added to your bank." : "Your guess was wrong... Better luck next time!";
          event.edit_original_response(
            dpp::message(
              std::format(
                "Wager: {}\nBet: Exact Roll\nNumber: {}\nTotal Coins: {}\n\n:game_die: Die landed on {}!\n{}",
                wager, number, user.coins, side, msg
              )
            )
          );
          
          try
          {
            pqxx::work trans(*this->m_services->client);

            trans.exec(R"(
                UPDATE guildmember SET coins = $1
                WHERE memberid = $2 AND guildid = $3;
              )",
              pqxx::params {
                user.coins,
                static_cast<uint64_t>(user_id), static_cast<uint64_t>(event.command.guild_id)
              }
            );
  
            trans.commit();
          }
          catch (const std::exception& e)
          {
            this->m_client->log(dpp::loglevel::ll_error, std::format("Could not update user {}", e.what()));
            return;
          }
        }).detach();
      }
      else if (subcmd.name == "evenodd")
      {
        if (this->is_empty(subcmd.options, event)) return;
        // TODO
      }
      else if (subcmd.name == "range")
      {
        if (this->is_empty(subcmd.options, event)) return;
        // TODO 
      }
    }
  private:
    bool is_empty(std::vector<dpp::command_data_option> options, const dpp::slashcommand_t& event)
    {
      if (options.empty())
      {
        dpp::embed emb = dpp::embed()
          .set_title("Uh oh!")
          .set_description("There's nobody here but us chickens! (You didn't provide required arguments.)")
          .set_colour(dpp::colours::pink)
          .set_timestamp(time(nullptr));

        event.reply(emb);
        return true;
      }
      
      return false;
    }
    
    dpp::cluster* m_client;
    std::shared_ptr<Services::SharedServices> m_services;
  };
}
