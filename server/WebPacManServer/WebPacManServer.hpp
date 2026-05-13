#pragma once

#include <thread>
#include <BS_thread_pool.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio; // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>


namespace pacman
{
    class Session;

    class WebPacManServer : public std::enable_shared_from_this<WebPacManServer>
    {
    public:
        WebPacManServer(const std::string& ipAddress, int port, const std::string& mazeFilePath,
                        int threadCount = std::thread::hardware_concurrency());

        ~WebPacManServer();

        bool run();

    private:
        std::string mazeFilePath;

        net::io_context io_context;

        tcp::acceptor acceptor;

        net::ip::address ipAddress;
        int port;

        tcp::endpoint endpoint;

        std::vector<std::shared_ptr<Session>> sessions;

        std::jthread gameTickerThread;
        int ioThreadCount;
        void gameTickerThreadKernel(std::stop_token stoken);

        void acceptSessions();

        void beginListening();

        void asyncAcceptHandler(beast::error_code ec, tcp::socket socket);
    };
} // pacman
