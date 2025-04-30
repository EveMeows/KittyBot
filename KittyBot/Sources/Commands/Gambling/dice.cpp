#include "Commands/Gambling/dice.h"

#include "Models/user.h"
#include "appcommand.h"
#include "dispatcher.h"
#include "Services/db.h"
#include "dpp/colors.h"
#include "dpp/message.h"
#include <random>

std::vector<dpp::command_option> Kitty::Commands::Gambling::Dice::options() const
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
  };
}

void Kitty::Commands::Gambling::Dice::execute(const dpp::slashcommand_t& event)
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
      std::uniform_int_distribution<int> distr(1, 6);
      
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
        user.coins += wager * 3;
      }

      std::string msg = won ? "You did it! Your wager has been *tripled* and added to your bank." : "Your guess was wrong... Better luck next time!";
      event.edit_original_response(
        dpp::message(
          std::format(
            "Wager: {}\nBet: Exact Roll\nNumber: {}\nTotal Coins: {}\n\n:game_die: Die landed on {}!\n{}",
            wager, number, user.coins, side, msg
          )
        )
      );

      update_user(user, static_cast<uint64_t>(user_id), static_cast<uint64_t>(event.command.guild_id));          
    }).detach();
  }
  else if (subcmd.name == "evenodd")
  {
    if (this->is_empty(subcmd.options, event)) return;
    
    long int wager = subcmd.get_value<long int>(0);
    if (wager < 1 || wager > user.coins)
    {
      event.reply("You cannot bet that amount!");
      return;
    }
    std::string type = subcmd.get_value<std::string>(1);
    
    event.reply(std::format("Wager: {}\nBet: Even/Odd Roll\nType: {}\nTotal Coins: {}\n\n:game_die: Rolling the die... Please hang on!", wager, type, user.coins));

    std::thread([type, user, wager, this, user_id, event]() mutable { 
      std::random_device rand;
      std::mt19937 rng(rand());
      std::uniform_int_distribution<int> distr(1, 6);
    
      int side = distr(rng);
      bool even = side % 2 == 0;

      using namespace std::chrono_literals;
      std::this_thread::sleep_for(2s);

      bool even_won = true;
      if (even)
      {
        if (type == "even")
        {
          user.coins += wager * 2;
        }
        else
        {
          even_won = false;
          user.coins -= wager;
        }
      }
      else
      {
        if (type == "even")
        {
          even_won = false;
          user.coins -= wager;
        }
        else
        {
          user.coins += wager * 2;
        }
      }
      
      std::string msg = even_won ? "You did it! Your wager has been doubled and added to your bank." : "Your guess was wrong... Better luck next time!";
      event.edit_original_response(
        dpp::message(
          std::format(
            "Wager: {}\nBet: Even/Odd Roll\nType: {}\nTotal Coins: {}\n\n:game_die: The die shows {}\n{}",
            wager, type, user.coins, even ? "Even" : "Odd", msg
          )
        )
      );

      this->update_user(user, static_cast<uint64_t>(user_id), static_cast<uint64_t>(event.command.guild_id));
    }).detach();
  }
}

void Kitty::Commands::Gambling::Dice::update_user(Models::KUser user, uint64_t user_id, uint64_t guild_id)
{
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

bool Kitty::Commands::Gambling::Dice::is_empty(std::vector<dpp::command_data_option> options, const dpp::slashcommand_t& event)
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
