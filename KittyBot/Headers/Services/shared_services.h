#pragma once

#include <memory>
#include <pqxx/pqxx>

namespace Kitty::Services
{
  struct SharedServices
  {
    std::unique_ptr<pqxx::connection> client = nullptr;
  };
} // namespace Kitty
