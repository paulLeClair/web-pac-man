cmake_minimum_required(VERSION "3.20")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        websocketpp
        GIT_REPOSITORY https://github.com/zaphoyd/websocketpp/websocketpp.git
        GIT_TAG 0.8.2
)

project(WebPacManServer CXX)

set(server_code
        main.cpp
        WebPacManServer/WebPacManServer.hpp
        WebPacManServer/WebPacManServer.cpp
)

add_executable(pacman_server WIN32 ${server_code})

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_DEBUG ON)

find_package(Boost 1.83.0 REQUIRED)
message(STATUS "Boost found: " ${BOOST_FOUND})

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(pacman_server Boost::dynamic_linking)
