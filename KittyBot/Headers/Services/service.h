#pragma once

#include <cpr/api.h>
#include <cpr/cpr.h>

#include "Services/service_response.h"

#include <cpr/response.h>
#include <format>
#include <string>

namespace Kitty::Services
{
  class Service
  {
  public:
    std::string url;

    std::string get() const
    {
      // JSON is provided by DPP.
      cpr::Url url = cpr::Url { this->url };
      cpr::Response res = cpr::Get(url);

      if (res.status_code != 200)
      {
        return std::format("{}", res.status_code);
      }

      return res.text;
    };

    virtual ServiceResponse fetch() = 0;
  };
}
