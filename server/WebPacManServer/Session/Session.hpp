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
        UserInputPress = 0x101,
        UserInputRelease = 0x102, // maybe unused
        GameModeComplete = 0x103,
    };

    enum class OutgoingPacketType : uint8_t
    {
        GameStateUpdate = 0x70,
        GameStateTransition = 0x80,
    };

    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        explicit Session(tcp::socket&& socket, const std::string &mapFilePath);

        ~Session() = default;

        boost::uuids::uuid sessionId;
        websocket::stream<beast::tcp_stream> ws;
        beast::flat_buffer incomingClientMessageBuffer = beast::flat_buffer();
        beast::flat_buffer gameStateFlatBuffer = beast::flat_buffer();

        void run();

        void gameTick();

    private:
        Game game;
        std::mutex mutex = std::mutex();

        // this is used to store the serialized game state data sent to the client
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
