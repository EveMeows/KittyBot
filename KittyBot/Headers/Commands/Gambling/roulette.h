#pragma once

#include "Commands/slash_command.h"
#include "Models/user.h"
#include "Services/db.h"
#include "Services/shared_services.h"
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/message.h>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <thread>
#include <vector>
namespace Kitty::Commands::Gambling
{
  class Roulette : public Commands::SlashCommand
  {
  public:
    Roulette(dpp::cluster *client,
             std::shared_ptr<Services::SharedServices> services)
        : Commands::SlashCommand("roulette",
                                 "Double your coin... But at what cost?")
    {
      this->m_client = client;
      this->m_services = services;
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

    void execute(const dpp::slashcommand_t &event) override
    {
      int64_t guild_id = event.command.guild_id;
      int64_t user_id = event.command.get_issuing_user().id;

      if (!Services::DB::guild_enrolled(this->m_services, guild_id))
      {
        event.reply("The guild isn't part of the database!");
        return;
      }

      std::optional<std::string> colour = this->param<std::string>("colour", event);
      if (!colour) return;

      std::optional<long int> wager = this->param<long int>("wager", event);
      if (!wager) return;

      Models::KUser user = Services::DB::ensure_user(this->m_services, user_id, guild_id);

      if (wager <= 0 || wager > user.coins)
      {
        event.reply("You can't do that!");
        return;
      }

      event.reply(std::format("Wager: {}\nBet: {}\nTotal Coins: {}\n\n🎰  Rolling roulette... Please Wait!", *wager, *colour == "roulette_red" ? "Red" : "Black", user.coins));

      std::thread(
        [user, wager = *wager, colour = *colour, this, event, guild_id, user_id]() mutable {
          using namespace std::chrono_literals;

          // Safe random int distribution lol
          std::random_device rand;
          std::mt19937 rng(rand());

          std::uniform_int_distribution<int> distr(0, 1);
          bool red = distr(rng);

          std::this_thread::sleep_for(3s);

          // Thread guard.
          std::unique_lock lock {this->m_wager_guard, std::try_to_lock};
          if (!lock)
          {
            event.edit_original_response(dpp::message("That's scary... I don't know..."));
            return;
          }

          if (colour == "roulette_red")
          {
            if (red)
            {
              user.coins += wager;
              event.edit_original_response(dpp::message(
                std::format(
                  "Wager: {}\nBet: Red\nTotal Coins: {}\n\n🎰 The ball landed on :red_square:! You doubled your wager!",
                  wager, user.coins
                ))
              );
            }
            else
            {
              user.coins -= wager;
              event.edit_original_response(dpp::message(
                std::format(
                  "Wager: {}\nBet: Red\nTotal Coins: {}\n\n🎰 The ball landed on :black_medium_square:! You lost... Better luck next time!",
                  wager, user.coins
                ))
              );
            }
          }
          else if (colour == "roulette_black")
          {
            if (!red)
            {
              user.coins += wager;
              event.edit_original_response(dpp::message(
                std::format(
                  "Wager: {}\nBet: Black\nTotal Coins: {}\n\n🎰 The ball landed on :black_medium_square:! You doubled your wager!",
                  wager, user.coins
                ))
              );
            }
            else
            {
              user.coins -= wager;
              event.edit_original_response(dpp::message(
                std::format(
                  "Wager: {}\nBet: Black\nTotal Coins: {}\n\n🎰 The ball landed on :red_square:! You lost... Better luck next time!",
                  wager, user.coins
                ))
              );
            }
          }

          // Update user...
          try
          {
            pqxx::work trans(*this->m_services->client);

            trans.exec(R"(
                UPDATE guildmember SET coins = $1
                WHERE memberid = $2 AND guildid = $3;
              )",
              pqxx::params {
                user.coins,
                user_id, guild_id
              }
            );
            
            trans.commit();
          }
          catch (const std::exception& e)
          {
            this->m_client->log(dpp::loglevel::ll_error, std::format("Could not update user {}", e.what()));
            return;
          }
        }
      ).detach();
    }

  private:
    std::mutex m_wager_guard;

    dpp::cluster *m_client;
    std::shared_ptr<Services::SharedServices> m_services;
  };
} // namespace Kitty::Commands::Gambling
