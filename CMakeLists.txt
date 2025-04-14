cmake_minimum_required(VERSION 3.15)
project(KittyBot VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

# Export compile_commands.json for clangd to see.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option (USE_EMBEDDED_DPP "Use embedded DPP." ON)
option (USE_EMBEDDED_CPR "Use embedded CPR." ON)
option (USE_EMBEDDED_PG "Use embedded C/C++ Connector." ON)

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
find_package(cpr QUIET)
if (NOT cpr_FOUND)
    if (USE_EMBEDDED_CPR)
        message(STATUS "CPR has not been found. Using Built-in.")

        # We use latest preview because why not :shrug:
        FetchContent_Declare(
            cpr
            GIT_REPOSITORY https://github.com/libcpr/cpr
            GIT_TAG master
        )

        FetchContent_MakeAvailable(cpr)
    else()
        message(SEND_ERROR
            "CPR has not been found and USE_EMBEDDED_CPR is disabled.
            Please install it on your system and try again."
        )
    endif()
else()
    message(STATUS "CPR has been found inside the system. Prioritising.")
endif()

# Postgre stuff
find_package(libpqxx QUIET)
if (NOT libpqxx_FOUND)
    if (USE_EMBEDDED_PG)
        message(STATUS "The C++ Connector has not been found inside the system. Using Built-in.")

        set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
        set(BUILD_STATIC_LIBS OFF CACHE BOOL "Build static libraries")
        set(SKIP_BUILD_TEST ON CACHE BOOL "Skip building tests")

        FetchContent_Declare(
            libpqxx
            GIT_REPOSITORY https://github.com/jtv/libpqxx
            GIT_TAG master
        )

        FetchContent_MakeAvailable(libpqxx)
    else()
        message(SEND_ERROR
            "The C++ Connector has not been found and USE_EMBEDDED_PG is disabled.
            Please install it on your system and try again."
        )
    endif()
else()
    message(STATUS "The C++ Connector has been found inside the system. Prioritising.")
endif()

add_executable(${PROJECT_NAME}
    KittyBot/Sources/main.cpp
    KittyBot/Sources/Services/nekosia.cpp
    KittyBot/Sources/Services/db_init.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    KittyBot/Headers
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    dpp cpr
    pqxx
)
