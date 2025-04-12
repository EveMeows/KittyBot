#pragma once

#include "Services/api_service.h"
#include "Services/api_service_response.h"

namespace Kitty::Services
{
  class Nekosia : public ApiService
  {
  public:
    Nekosia()
    {
      this->url = "https://api.nekosia.cat/api/v1/images/catgirl";
    }

    ApiServiceResponse fetch() override;
  };
} // namespace Kitty::Services
