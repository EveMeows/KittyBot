#pragma once

#include "Services/service.h"
#include "Services/service_response.h"
namespace Kitty::Services
{

  class Nekosia : public Service
  {
  public:
    Nekosia()
    {
      this->url = "https://api.nekosia.cat/api/v1/images/catgirl";
    }
    
    ServiceResponse fetch() override;
  };
}
