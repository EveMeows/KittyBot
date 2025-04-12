#pragma once

#include "Commands/slash_command.h"
#include "Services/api_service.h"
#include "Services/api_service_response.h"
#include "Services/nekosia.h"
#include "Services/shared_services.h"

#include <dpp/cluster.h>
#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <memory>
#include <ctime>

namespace Kitty::Commands
{
  class Kitty : public SlashCommand
  {
  private:
    dpp::cluster *m_client;
    std::unique_ptr<Services::ApiService> m_api_service;

  public:
    Kitty(dpp::cluster *client, std::shared_ptr<Services::SharedServices> services)
        : SlashCommand("kitty", "Fetch a silly little picture!")
    {
      this->m_client = client;
      this->m_api_service = std::make_unique<Services::Nekosia>();
    }

    void execute(const dpp::slashcommand_t &event) override
    {
      Services::ApiServiceResponse res = this->m_api_service->fetch();

      dpp::user user = event.command.get_issuing_user();

      dpp::embed embed =
          dpp::embed()
              .set_colour(dpp::colours::pink)
              .set_title("Here's your kitty!")
              .set_image(res.url)
              .set_author(std::format("Art made by {}.", res.artist),
                          res.artist_url, "")
              .set_footer(dpp::embed_footer()
                              .set_text(user.username)
                              .set_icon(user.get_avatar_url()))
              .set_timestamp(time(nullptr));

      event.reply(embed);
    }
  };
} // namespace Kitty::Commands
