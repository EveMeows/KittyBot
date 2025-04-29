#include "Commands/ping.h"
#include "dpp/appcommand.h"
#include <optional>

void Kitty::Commands::Ping::execute(const dpp::slashcommand_t& event)
{
  std::string res = "Pong! I am made in C++.";
  if (std::optional<bool> ping = this->param<bool>("ping", event))
  {
    if (*ping)
    {
      res = std::format(
        "Pong! I am made in C++.\nMy ping is approx. {:.3f}ms!",
        this->m_client->rest_ping
      );
    }
  }

  event.reply(res);
}

std::vector<dpp::command_option> Kitty::Commands::Ping::options() const
{
  return {
    dpp::command_option(dpp::co_boolean, "ping", "Show bot ping.", false)
  };
}
