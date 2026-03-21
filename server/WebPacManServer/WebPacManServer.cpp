//
// Created by paull on 2026-01-12.
//

#include "WebPacManServer.hpp"

#include "Session/Session.hpp"

#include <boost/asio/strand.hpp>

namespace pacman
{
    WebPacManServer::WebPacManServer(const std::string& ip, const int port, const std::string& mazeFilePath,
                                     const int threadCount)
        : mazeFilePath(mazeFilePath), // i guess default for now is 50% of threads go to game logic
          ioThreadCount(threadCount),
          io_context(threadCount),
          acceptor(io_context),
          ipAddress(boost::asio::ip::address::from_string(ip)),
          port(port),
          endpoint(ipAddress,
                   static_cast<const uint8_t>(static_cast<uint8_t>(port)))
    {
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

    bool WebPacManServer::run()
    {
        acceptSessions();

        std::vector<std::thread> serverThreads = {};
        serverThreads.reserve(ioThreadCount);
        for (uint32_t i = 0; i < ioThreadCount; i++)
        {
            serverThreads.emplace_back([&]
            {
                io_context.run();
            });
        }

        const auto& gameTickerFunction = [&](const std::stop_token& stoken)
        {
            gameTickerThreadKernel(stoken);
        };
        gameTickerThread = std::jthread(gameTickerFunction);

        try
        {
            io_context.run(); // bug: weird exception throws here but doesn't seem to block anything happening
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }

        gameTickerThread.join();
        return true;
    }


    void WebPacManServer::acceptSessions()
    {
        beast::error_code ec;

        auto result = acceptor.open(endpoint.protocol(), ec);
        if (ec)
        {
            //TODO -> log failure!
            return;
        }
        if (result)
        {
            return;
        }

        result = acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
        if (ec)
        {
            // TODO -> log failure!
            return;
        }
        if (result)
        {
            return;
        }

        result = acceptor.bind(endpoint, ec);
        if (ec)
        {
            // TODO -> log failure!
            return;
        }
        if (result)
        {
            return;
        }

        result = acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec)
        {
            // TODO -> log failure!
            return;
        }
        if (result)
        {
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
            sessions.emplace_back(std::make_shared<Session>(
                std::move(socket),
                mazeFilePath
            ));
            sessions.back()->run();
        }

        beginListening();
    }


    void WebPacManServer::gameTickerThreadKernel(std::stop_token stoken) //NOLINT
    {
        static constexpr int DEFAULT_TICK_INTERVAL_IN_MS = 20;
        while (!stoken.stop_requested())
        {
            for (const auto& session : sessions)
            {
                session->gameTick();
            }

            // TODO -> replace this ugly sleeping with a condition variable for more efficient waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_TICK_INTERVAL_IN_MS));
        }
    }
} // pacman
