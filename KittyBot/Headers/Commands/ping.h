#pragma once

#include "Commands/slash_command.h"

#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/event.h>
#include <dpp/misc-enum.h>

#include <optional>

namespace Kitty::Commands
{
    class Ping : public SlashCommand
    {
    private:
        dpp::cluster* m_client;
    public:
        Ping(dpp::cluster* client) : SlashCommand("ping", "Pong!")
        {
            this->m_client = client;
        };

        std::vector<dpp::command_option> options() const override {
            return {
                dpp::command_option(dpp::co_boolean, "ping", "Show bot ping.", false)
            };
        }

        void execute(const dpp::slashcommand_t& event) override
        {
            std::string res = "Pong! I am made in C++.";
            if (std::optional<bool> ping = this->param<bool>("ping", event))
            {
                if (*ping)
                {
                    res = std::format(
                        "Pong! I am made in C++.\n My ping is approx. {:.3f}ms!",
                        this->m_client->rest_ping
                    );
                }
            }

            event.reply(res);
        }
    };
}
