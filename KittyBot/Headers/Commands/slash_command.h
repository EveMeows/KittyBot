#pragma once

#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>

#include <optional>
#include <string>

namespace Kitty::Commands
{
  class SlashCommand
  {
  public:
    const std::string name, description;
    SlashCommand(const std::string name, const std::string description) : name(name), description(description) {};

    // ???
    virtual ~SlashCommand() = default;

    template <typename T>
    std::optional<T> param(const std::string name, const dpp::slashcommand_t &event) const
    {
      const dpp::command_value &param = event.get_parameter(name);
      if (param.index() != 0)
      {
        try
        {
          return std::get<T>(param);
        }
        catch (const std::bad_variant_access &)
        {
          return std::nullopt;
        }
      }

      return std::nullopt;
    }

    virtual void execute(const dpp::slashcommand_t &event) = 0;
    virtual std::vector<dpp::command_option> options() const;
    virtual dpp::slashcommand get_dpp_command(dpp::snowflake bot_id) const;
  protected:
    /**
    * @brief The required permissions for the command, by default everyone is allowed.
    * @note This must be set ONLY in the constructor. Nothing will happen if the command is already registered.
    */
    dpp::permission m_permissions = 0;
  };
} // namespace Kitty::Commands
