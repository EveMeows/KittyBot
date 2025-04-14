#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <exception>
#include <format>
#include <memory>
namespace Kitty::Commands::Administrative
{
  class Enroll : public Commands::SlashCommand
  {
  public:
    Enroll(dpp::cluster* client, std::shared_ptr<Services::SharedServices> shared)
      : Commands::SlashCommand("enroll", "Enroll the server into the database.")
    {
      this->m_shared = shared;
      this->m_client = client;

      this->m_permissions = dpp::permissions::p_administrator;
    }

    void execute(const dpp::slashcommand_t& event) override;
  private:
    std::shared_ptr<Services::SharedServices> m_shared;
    dpp::cluster* m_client = nullptr;
  };

  inline void Enroll::execute(const dpp::slashcommand_t& event)
  {
    try
    {
      pqxx::work insert(*this->m_shared->client);

      // Check if the guild is already in the database
      auto result = insert.exec_params("SELECT id FROM guild WHERE id = $1;", static_cast<int64_t>(event.command.guild_id));
      if (!result.empty())
      {
        event.reply("Your server is already enrolled into my database!");
        return;
      }

      // Insert the guild into the database.
      insert.exec_params("INSERT INTO guild (id) VALUES ($1);", static_cast<int64_t>(event.command.guild_id));

      insert.commit();
    }
    catch (const std::exception& e)
    {
      this->m_client->log(dpp::loglevel::ll_error, std::format("Could not enroll server inside db: {}", e.what()));
      event.reply("Your server could not be enrolled into the database...");

      return;
    }

    dpp::embed thanks = dpp::embed()
      .set_colour(dpp::colours::pink)
      .set_title("Thank you!")
      .set_description("Your server is now enrolled into my database!\nYour users can now level up and access the economy!");

    event.reply(thanks);
  }
}
