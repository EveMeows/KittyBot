#pragma once

#include <string>
namespace Kitty::Models
{
  struct KGuild
  {
    std::string note_prefix;
    int note_minlevel;
    bool note_allow;
  };
}
