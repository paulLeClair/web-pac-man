#include <iostream>

#include "WebPacManServer/WebPacManServer.hpp"


// TODO -> simple linux implementation
#ifdef _WIN32

#include <Windows.h>

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
    // TODO -> get ip & port from cmd line arg
    std::string defaultIP = "127.0.0.1";
    int defaultPort = 80; // ?

    const auto server = std::make_shared<pacman::WebPacManServer>(defaultIP, defaultPort);

    server->run();

    return 0;
}


#endif
