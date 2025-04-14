#pragma once

#include "Services/shared_services.h"
#include <memory>

namespace Kitty::Services::DB::Init
{
  bool drop_base_tables(std::shared_ptr<Services::SharedServices> shared);
  bool create_base_tables(std::shared_ptr<Services::SharedServices> shared);
}
