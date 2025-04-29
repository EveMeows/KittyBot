#include "Commands/kitty.h"
#include "Services/api_service_response.h"

void Kitty::Commands::Kitty::execute(const dpp::slashcommand_t& event)
{
  Services::ApiServiceResponse res = this->m_api_service->fetch();

  dpp::user user = event.command.get_issuing_user();

  dpp::embed embed = dpp::embed()
    .set_colour(dpp::colours::pink)
    .set_title("Here's your kitty!")
    .set_image(res.url)
    .set_author(std::format("Art made by {}.", res.artist), res.artist_url, "")
    .set_footer(dpp::embed_footer()
      .set_text(user.username)
      .set_icon(user.get_avatar_url()))
    .set_timestamp(time(nullptr));

  event.reply(embed);
}
