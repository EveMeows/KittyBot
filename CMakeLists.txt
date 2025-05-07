cmake_minimum_required(VERSION 3.15)
project(KittyBot VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option (USE_EMBEDDED_DPP "Use embedded DPP." ON)

# Copy .env to output.
file(COPY ".env" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

include(FetchContent)

# DPP (Discord Client Library).
find_package(DPP QUIET)
if (NOT DPP_FOUND)
    if (USE_EMBEDDED_DPP)
        message(STATUS "DPP (The Discord API wrapper) has not been found. Using Built-in.")

        FetchContent_Declare(
            dpp
            GIT_REPOSITORY https://github.com/brainboxdotcc/DPP
            GIT_TAG master
        )

        FetchContent_MakeAvailable(dpp)
    else()
        message(SEND_ERROR
            "DPP (The Discord API wrapper) has not been found and USE_EMBEDDED_DPP is disabled.
            Please install it on your system by following this link: \"https://dpp.dev/installing.html\""
        )
    endif()
else()
    message(STATUS "DPP (The Discord API wrapper) has been found inside the system. Prioritising.")
endif()

# CPR (Curl For People) C++ edition lol
find_package(cpr CONFIG REQUIRED)

# Postgre stuff
find_package(libpqxx CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    KittyBot/Sources/main.cpp

    # Services
    KittyBot/Sources/Services/nekosia.cpp

    KittyBot/Sources/Services/db_init.cpp
    KittyBot/Sources/Services/db.cpp

    # Commands
    KittyBot/Sources/Commands/manager.cpp
    KittyBot/Sources/Commands/slash_command.cpp

    KittyBot/Sources/Commands/Gambling/dice.cpp
    KittyBot/Sources/Commands/Gambling/roulette.cpp

    KittyBot/Sources/Commands/User/uinfo.cpp

    KittyBot/Sources/Commands/Administrative/enroll.cpp
    KittyBot/Sources/Commands/Administrative/add_coins.cpp
    KittyBot/Sources/Commands/Administrative/remove_coins.cpp
    KittyBot/Sources/Commands/Administrative/add_xp.cpp
    KittyBot/Sources/Commands/Administrative/notes_admin.cpp

    KittyBot/Sources/Commands/ping.cpp
    KittyBot/Sources/Commands/kitty.cpp
    KittyBot/Sources/Commands/notes.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    KittyBot/Headers
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    dpp cpr::cpr
    libpqxx::pqxx
)
