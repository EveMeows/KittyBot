#pragma once

#include <cpr/api.h>
#include <cpr/cpr.h>

#include "Services/api_service_response.h"

#include <cpr/response.h>
#include <format>
#include <string>

namespace Kitty::Services
{
  class ApiService
  {
  public:
    std::string url;

    std::string get() const
    {
      // JSON is provided by DPP.
      cpr::Url url = cpr::Url{this->url};
      cpr::Response res = cpr::Get(url);

      if (res.status_code != 200)
      {
        return std::format("{}", res.status_code);
      }

      return res.text;
    };

    virtual ApiServiceResponse fetch() = 0;
  };
} // namespace Kitty::Services
