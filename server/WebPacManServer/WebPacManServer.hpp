//
// Created by paull on 2026-01-12.
//

#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace pacman {
    class Session;

    class WebPacManServer {
public:
        WebPacManServer(const std::string &ipAddress, int port);

        ~WebPacManServer();

        bool start();

private:
        net::io_context io_context{1};

        tcp::acceptor acceptor;

        std::string ipAddress;
        int port;

        /**
         * The server will just create 1 socket per active session
         */
        std::vector<std::unique_ptr<Session>> sessions;

        void listenForSessions();
    };

} // pacman
