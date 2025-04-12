#include "Services/nekosia.h"
#include "Services/api_service_response.h"

#include <dpp/nlohmann/json.hpp>

Kitty::Services::ApiServiceResponse Kitty::Services::Nekosia::fetch()
{
  std::string json_string = this->get();
  nlohmann::json json = nlohmann::json::parse(json_string);

  Kitty::Services::ApiServiceResponse resp(
      json["image"]["original"]["url"],
      json["attribution"]["artist"]["username"],
      json["attribution"]["artist"]["profile"]);
  return resp;
}
