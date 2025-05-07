#pragma once

#include "Models/guild.h"
#include "Models/user.h"
#include "Services/shared_services.h"
#include <dpp/snowflake.h>
#include <memory>
#include <optional>
namespace Kitty::Services::DB
{
  Models::KUser ensure_user(std::shared_ptr<Services::SharedServices> services, dpp::snowflake member_id, dpp::snowflake guild_id);

  bool guild_enrolled(std::shared_ptr<Services::SharedServices> services, dpp::snowflake guild_id);
  std::optional<Models::KGuild> maybe_guild(std::shared_ptr<Services::SharedServices> services, dpp::snowflake guild_id);
}
