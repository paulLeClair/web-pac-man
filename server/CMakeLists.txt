cmake_minimum_required(VERSION "3.20")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)


project(WebPacManServer CXX)

set(server_code
        main.cpp
        WebPacManServer/WebPacManServer.hpp
        WebPacManServer/WebPacManServer.cpp
        WebPacManServer/Session/Session.cpp
        WebPacManServer/Session/Session.hpp
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
target_link_libraries(pacman_server ws2_32)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
FetchContent_Declare(
        bshoshany_thread_pool
        GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
        GIT_TAG v5.1.0
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        EXCLUDE_FROM_ALL
        SYSTEM
)
FetchContent_MakeAvailable(bshoshany_thread_pool)
add_library(BS_thread_pool INTERFACE)
target_include_directories(BS_thread_pool INTERFACE ${bshoshany_thread_pool_SOURCE_DIR}/include)

target_link_libraries(pacman_server BS_thread_pool)
