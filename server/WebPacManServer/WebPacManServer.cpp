//
// Created by paull on 2026-01-12.
//

#include "WebPacManServer.hpp"

#include "Session/Session.hpp"

#include <boost/asio/strand.hpp>

namespace pacman {
    WebPacManServer::WebPacManServer(const std::string &ip, const int port, const int threadCount)
    : gameLogicThreadPool(threadCount / 2), // i guess default for now is 50% of threads go to game logic
    ipAddress(boost::asio::ip::address::from_string(ip)),
    port(port),
    io_context(threadCount / 2),
    endpoint(ipAddress,
            static_cast<const uint8_t>(static_cast<uint8_t>(port))),
    acceptor(io_context) {
    }

    WebPacManServer::~WebPacManServer()
    {
        if (!gameTickerThread.request_stop())
        {
            // TODO -> log error!
            return;
        }
        gameTickerThread.join();
    }

    bool WebPacManServer::start()
    {
        const auto &wtf = [&](std::stop_token stoken)
        {
            gameTickerThreadKernel(stoken);
        };
        gameTickerThread = std::jthread(wtf);

        acceptSessions();
        return true;
    }


    void WebPacManServer::acceptSessions()
    {
        beast::error_code ec;

        acceptor.open(endpoint.protocol(), ec);
        if (ec)
        {
            //TODO -> log failure!
            return;
        }

        acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec)
        {
            // TODO -> log failure!
            return;
        }

        acceptor.bind(endpoint, ec);
        if (ec)
        {
            // TODO -> log failure!
            return;
        }

        acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            // TODO -> log failure!
            return;
        }

        beginListening();
    }

    void WebPacManServer::beginListening()
    {
        acceptor.async_accept(
            net::make_strand(io_context),
            beast::bind_front_handler(&WebPacManServer::asyncAcceptHandler, shared_from_this())
        );
    }

    void WebPacManServer::asyncAcceptHandler(beast::error_code ec, tcp::socket socket)
    {
        if (ec)
        {
            // TODO -> log failure!
        }
        else
        {
            // we can start a new session
            sessions.emplace_back(std::make_unique<Session>(
                std::move(socket)
            ));
            sessions.back()->run();
        }

        beginListening();
    }


    void WebPacManServer::gameTickerThreadKernel(std::stop_token stoken)
    {
        static constexpr int DEFAULT_TICK_INTERVAL_IN_MS = 15;
        while (!stoken.stop_requested())
        {
            for (auto &session : sessions)
            {
                auto result = gameLogicThreadPool.submit_task([&]{ session->gameTick(); }); // NOLINT

            }

            std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TICK_INTERVAL_IN_MS));
        }
    }
} // pacman