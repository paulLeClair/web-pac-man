#include <iostream>

#include "WebPacManServer/WebPacManServer.hpp"

#ifdef _WIN32

#include <Windows.h>


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {

    pacman::WebPacManServer server;

    server.start();

    return 0;
}


#endif
