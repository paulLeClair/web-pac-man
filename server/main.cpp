#include <iostream>

#include "WebPacManServer/WebPacManServer.hpp"


// TODO -> simple linux implementation
#ifdef _WIN32

#include <Windows.h>
#include <shellapi.h>
#include <locale>
#include <codecvt>

#include "WebPacManServer/Session/Session.hpp"

// convenience function to enable usage of CommandLineToArgvW
std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>; // NOLINT
    std::wstring_convert<convert_typeX> converterX; // NOLINT

    return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>; // NOLINT
    std::wstring_convert<convert_typeX> converterX; // NOLINT

    return converterX.to_bytes(wstr);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) { // NOLINT
    const auto lpwCmdLine = s2ws(lpCmdLine);

    int argc;
    const LPWSTR* argv = CommandLineToArgvW(lpwCmdLine.c_str(), &argc);

    if (argc != 3)
    {
        std::cout << "Usage: <server ip> <server port> <maze file path>" << std::endl;
        return 0;
    }

    std::string defaultIP = ws2s(argv[0]);
    int defaultPort = std::stoi(ws2s(argv[1]));
    std::string mazeFilePath = ws2s(argv[2]);

    const auto server = std::make_shared<pacman::WebPacManServer>(defaultIP, defaultPort, mazeFilePath);
    server->run();

    return 0;
}


#endif
