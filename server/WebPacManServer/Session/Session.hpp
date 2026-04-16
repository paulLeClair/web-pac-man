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
        explicit Session(tcp::socket&& socket, const std::string& mapFilePath);

        ~Session() = default;

        boost::uuids::uuid sessionId;
        websocket::stream<beast::tcp_stream> ws;
        beast::flat_buffer incomingClientMessageBuffer = beast::flat_buffer(); // TODO -> get rid of this too

        void run();

        void gameTick();

        std::mutex mutex = std::mutex();

    private:
        friend class Game; // temp

        std::shared_ptr<Game> game = nullptr;

        // this is used to store the serialized game state data sent to the client

        // likely it will be good to add more explicit sync here, including an "outgoing sound packets" queue
        // that might be worth generalizing for all outgoing packets
        bool writeInProgress = false;
        std::deque<std::shared_ptr<std::vector<uint8_t>>> outgoingWpmPackets;
        std::deque<std::shared_ptr<std::vector<uint8_t>>> outgoingGameStatePackets;

        // TODO -> have a queue for outgoing game state updates as well

        void asyncRunHandler();
        void listenToClient();
        void asyncWebsocketAcceptHandler(beast::error_code ec);
        void asyncListenToClientHandler(beast::error_code ec, std::size_t bytes_transferred);

        void handleTextMessage();
        /**
         *
         * @param direction1 a direction
         * @param direction2 a direction
         * @return whether direction1 is the opposite direction of direction2
         */
        static bool isOppositeDirection(Direction direction1, Direction direction2);
        void handleBinaryMessage();
        void sendWpmPacket(WpmPacketType packetType, int32_t payload);
        void writeWpmPacket();
        void writeGameStatePacket();
    };
} // pacman
