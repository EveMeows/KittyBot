#pragma once

#include <string>
namespace Kitty::Models
{
  struct KUser
  {
    int coins, xp, xpstep, xpnext, level;
    std::string lastclaimed;
  };
}
