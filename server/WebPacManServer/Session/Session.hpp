//
// Created by paull on 2026-01-15.
//

#pragma once

#include "../WebPacManServer.hpp"

#include "Game/Game.hpp"

#include "boost/uuid/uuid.hpp"

namespace pacman
{
    enum class IncomingPacketType : int32_t
    {
        UserInputPress,
        UserInputRelease, // maybe unused
        GameModeComplete
    };

    enum class OutgoingPacketType : uint8_t
    {
        GameStateUpdate = 0x70,
        GameStateTransition = 0x80,
    };

    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        explicit Session(tcp::socket&& socket);

        ~Session() = default;

        boost::uuids::uuid sessionId;
        websocket::stream<beast::tcp_stream> ws;
        beast::flat_buffer incomingClientMessageBuffer = beast::flat_buffer();
        beast::flat_buffer gameStateFlatBuffer = beast::flat_buffer();

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
        std::mutex mutex = std::mutex();

        // this is used to store the serialized game state data that is sent to the client
        std::vector<uint8_t> gameStateMessageData;

        void asyncRunHandler();
        void listenToClient();
        void asyncAcceptHandler(beast::error_code ec);
        void asyncListenToClientHandler(beast::error_code ec, std::size_t bytes_transferred);
        void asyncTestEchoInputHandler(beast::error_code ec, std::size_t bytes_transferred);
        void asyncWriteGameStateHandler(beast::error_code ec, std::size_t bytes_transferred);

        void handleTextMessage();
        void handleBinaryMessage();
    };
} // pacman
