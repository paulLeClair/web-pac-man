//
// Created by paull on 2026-01-15.
//

#pragma once

#include "../WebPacManServer.hpp"

#include "Game/Game.hpp"

#include "boost/uuid/uuid.hpp"

namespace pacman
{
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        explicit Session(tcp::socket &&socket);

        ~Session() = default;

        boost::uuids::uuid sessionId;
        websocket::stream<beast::tcp_stream> ws;
        beast::flat_buffer userInputBuffer = beast::flat_buffer();
        beast::flat_buffer gameStateBuffer= beast::flat_buffer();

        // i'll try and tailor this to use boost asio strands, which should hopefully in conjunction with the
        // thread pool allow for a fully multithreaded C++ server

        // one thing we have to reconcile is that we need to tie in ticking each session's game at a
        // reasonable rate with the async input reads and async game state writes...


        // triggers the session to begin
        void run();

        // for now my only idea for a semi-clean design is to spin up a server thread that
        // runs updates for all sessions (using a thread pool to hopefully mitigate
        // lag from game updates when scaling up to large numbers of sessions)
        void gameTick();


    private:
        Game game;

        void asyncRunHandler();
        void readUserInputs();
        void asyncAcceptHandler(beast::error_code ec);
        void asyncReadUserInputHandler(beast::error_code ec, std::size_t bytes_transferred);
        void asyncTestEchoInputHandler(beast::error_code ec, std::size_t bytes_transferred);
        void asyncWriteGameStateHandler(beast::error_code ec, std::size_t bytes_transferred);
    };
} // pacman
