#pragma once

#include <string>
namespace Kitty::Services
{
  struct ApiServiceResponse
  {
    const std::string url, artist, artist_url;

    ApiServiceResponse(const std::string url, const std::string artist,
                       const std::string artist_url)
        : url(url), artist(artist), artist_url(artist_url) {};
  };
} // namespace Kitty::Services
