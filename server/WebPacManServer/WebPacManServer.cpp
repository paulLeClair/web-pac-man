//
// Created by paull on 2026-01-12.
//

#include "Session/Session.hpp"
#include "WebPacManServer.hpp"

#include <thread>

namespace pacman {
    WebPacManServer::WebPacManServer(const std::string &ipAddress, int port)
    : ipAddress(ipAddress), port(port), acceptor(tcp::acceptor{
        io_context,
        {
            (boost::asio::ip::address::from_string(ipAddress)),
            static_cast<const uint8_t>(static_cast<uint8_t>(port))
        }
    }) {

    }

    WebPacManServer::~WebPacManServer() = default;

    // this is kinda weird so i should redo the base interface
    bool WebPacManServer::start()
    {
        listenForSessions();
        return true;
    }

    void WebPacManServer::listenForSessions()
    {
        int numSessions = 0;
        while (numSessions < 100) // arbitrary upper bound on sessions for now
        {
           auto newSocket = tcp::socket{io_context};

            // wait for connection
            acceptor.accept(newSocket);

            // create new session
            sessions.emplace_back(std::make_unique<Session>(std::move(newSocket)));

            numSessions++;
        }
    }
} // pacman