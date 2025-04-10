#pragma once

#include "Commands/slash_command.h"
#include "Services/nekosia.h"
#include "Services/service.h"
#include "Services/service_response.h"

#include <ctime>
#include <dpp/cluster.h>

#include <dpp/colors.h>
#include <dpp/dispatcher.h>
#include <memory>

namespace Kitty::Commands
{
  class Kitty : public SlashCommand
  {
  private:
    dpp::cluster* m_client;
    std::unique_ptr<Services::Service> m_service;

  public:
    Kitty(dpp::cluster* client) : SlashCommand("kitty", "Fetch a silly little picture!")
    {
      this->m_client = client;
      this->m_service = std::make_unique<Services::Nekosia>();
    }

    void execute(const dpp::slashcommand_t& event) override
    {
      Services::ServiceResponse res = this->m_service->fetch();

      dpp::user user = event.command.get_issuing_user();

      dpp::embed embed = dpp::embed()
        .set_colour(dpp::colours::pink)
        .set_title("Here's your kitty!")
        .set_image(res.url)
        .set_author(
            std::format("Art made by {}.", res.artist),
            res.artist_url, ""
        )
        .set_footer(
          dpp::embed_footer()
            .set_text(user.username)
            .set_icon(user.get_avatar_url())
        )
        .set_timestamp(time(nullptr));
      
      event.reply(embed);
    }
  };
}
