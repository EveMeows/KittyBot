#pragma once

#include "Commands/slash_command.h"
#include "Services/shared_services.h"
#include "appcommand.h"
#include "cluster.h"
#include <memory>

namespace Kitty::Commands::Administrative
{
  class ModNotes : public Commands::SlashCommand
  {
  public:
    ModNotes(dpp::cluster* client, std::shared_ptr<Services::SharedServices> services)
      : Commands::SlashCommand("modnotes", "Manage the server's note options!")
    {
      this->m_permissions = dpp::permissions::p_administrator;

      this->m_client = client;
      this->m_services = services;
    };

    std::vector<dpp::command_option> options() const override;
    void execute(const dpp::slashcommand_t& event) override;
  private:
    dpp::cluster* m_client;
    std::shared_ptr<Services::SharedServices> m_services;

    // Commands
    void creation(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd);
    void level(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd);
    void prefix(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd);
    void stats(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd);

    void erase(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd);
    void edit(const dpp::slashcommand_t& event, dpp::command_data_option& subcmd);
  };
}
