#pragma once

#include "util.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace Kitty
{
    /// @brief parse a .env file, in the root directory.
    inline void parse_env()
    {
        const std::string base = Kitty::Util::get_base_directory();
        if (base == "")
        {
            std::cerr << "ERROR: PARSE_ENV: Cannot get base binary path.\n";
            return;
        }

        const std::filesystem::path path = base / std::filesystem::path(".env");

        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cerr << "ERROR: PARSE_ENV: Cannot find any .env file in binary directory.\n";
            return;
        }

        std::string ln;
        while (std::getline(file, ln))
        {
            if (ln.empty() || ln[0] == '#') continue;

            unsigned long del = ln.find('=');
            if (del == std::string::npos)
            {
                std::cerr  << "ERROR: PARSE_ENV: No delimiter (=) found.\n";
                continue;
            }

            std::string key = ln.substr(0, del);
            std::string value = ln.substr(del + 1, ln.size());

            if (key.empty() || value.empty())
            {
                std::cerr << "ERROR: PARSE_ENV: No key or value provided.\n";
                continue;
            }

            // 1 = replace key if exists.
            setenv(key.c_str(), value.c_str(), 1);
        }

        file.close();
    }
}
