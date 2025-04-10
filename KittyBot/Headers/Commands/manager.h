#pragma once

#include "Commands/slash_command.h"

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/misc-enum.h>

#include <format>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace Kitty::Commands
{
    class CommandManager
    {
    private:
        std::vector<std::unique_ptr<SlashCommand>> m_commands;
        dpp::cluster* m_client = nullptr;

    public:
        CommandManager(dpp::cluster* client) : m_client(client) {};

        template<typename TCommand, typename... TArgs>
        void enroll(TArgs&&... args)
        {
            static_assert(std::is_base_of<SlashCommand, TCommand>::value, "TCommand must be derived from SlashCommand.");

            std::unique_ptr<TCommand> command = std::make_unique<TCommand>(this->m_client, std::forward<TArgs>(args)...);
            this->m_commands.emplace_back(std::move(command));
        }

        /// @brief Create all the slash commands stored in the manager.
        void create_all()
        {
            for (const std::unique_ptr<SlashCommand>& command : this->m_commands)
            {
                this->m_client->global_command_create(
                    command->get_dpp_command(this->m_client->me.id)
                );

                this->m_client->log(dpp::loglevel::ll_info, std::format("Created command {}.", command->name));
            }
        }

        /// @brief Handle slash commands when a command is executed.
        void handle(const dpp::slashcommand_t& event)
        {
            for (std::unique_ptr<SlashCommand>& command : this->m_commands)
            {
                if (event.command.get_command_name() == command->name)
                {
                    command->execute(event);
                    return;
                }
            }
        }
    };
}
