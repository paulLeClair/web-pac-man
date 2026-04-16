//
// Created by paull on 2026-01-15.
//

#include "Session.hpp"

#include "../../protobuf/gen/game_state.pb.h"
#include "../../protobuf/gen/user_inputs.pb.h"
#include "../../protobuf/gen/wpm_packet.pb.h"

namespace pacman
{
    Session::Session(tcp::socket&& socket, const std::string& mapFilePath) : sessionId(boost::uuids::uuid()),
                                                                             ws(std::move(socket)),
                                                                             game(std::make_shared<Game>(mapFilePath))
    {
    }

    /**
     * This is the thread kernel for a particular pacman game session
     * @return An enum to describe whether the session exited with an error or was successful
     */
    void Session::run()
    {
        net::dispatch(ws.get_executor(),
                      beast::bind_front_handler(&Session::asyncRunHandler, shared_from_this()));
    }

    void Session::gameTick()
    {
        std::lock_guard guard(mutex);
        game->tick(*this);

        // for now naive flat update of all state variables at once
        GameStateMessage gameState;

        gameState.set_score(game->score);
        gameState.set_level(game->level);
        gameState.set_ghostsarescattering(game->ghostsAreScattering);

        // player update
        gameState.set_pacmanischomping(game->player.isChomping);
        gameState.set_pacmanorientation(static_cast<int32_t>(game->player.orientation));
        gameState.set_pacmanpositionx(game->player.pos.x);
        gameState.set_pacmanpositiony(game->player.pos.y);
        gameState.set_pacmanishidden(game->player.hidden);

        // ghost updates
        gameState.set_blinkyisdead(game->blinky.isDead);
        gameState.set_blinkyorientation(static_cast<int32_t>(game->blinky.orientation));
        gameState.set_blinkypositionx(game->blinky.pos.x);
        gameState.set_blinkypositiony(game->blinky.pos.y);
        gameState.set_blinkyishidden(game->blinky.hidden);

        gameState.set_inkyisdead(game->inky.isDead);
        gameState.set_inkyorientation(static_cast<int32_t>(game->inky.orientation));
        gameState.set_inkypositionx(game->inky.pos.x);
        gameState.set_inkypositiony(game->inky.pos.y);
        gameState.set_inkyishidden(game->inky.hidden);

        gameState.set_pinkyisdead(game->pinky.isDead);
        gameState.set_pinkyorientation(static_cast<int32_t>(game->pinky.orientation));
        gameState.set_pinkypositionx(game->pinky.pos.x);
        gameState.set_pinkypositiony(game->pinky.pos.y);
        gameState.set_pinkyishidden(game->pinky.hidden);

        gameState.set_clydeisdead(game->clyde.isDead);
        gameState.set_clydeorientation(static_cast<int32_t>(game->clyde.orientation));
        gameState.set_clydepositionx(game->clyde.pos.x);
        gameState.set_clydepositiony(game->clyde.pos.y);
        gameState.set_clydeishidden(game->clyde.hidden);

        gameState.set_pacmanisdead(game->pacmanIsDead);
        gameState.set_displayreadymessage(game->displayReadyMessage);
        gameState.set_displayattractmessage(game->displayAttractMessage);
        gameState.set_displaygameovermessage(game->displayGameOverMessage);

        gameState.clear_items();
        const auto itemsHandle = gameState.mutable_items();
        for (auto& [packedCoords, type] : game->items)
        {
            const uint32_t pixelwiseX = (packedCoords >> 16) * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS;
            const uint32_t pixelwiseY = ((packedCoords << 16) >> 16) * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS;

            uint32_t pixelwisePackedCoords = pixelwiseX << 16 | pixelwiseY;
            itemsHandle->insert(std::make_pair(
                pixelwisePackedCoords,
                static_cast<uint32_t>(type)));
        }

        std::vector<uint8_t> gameStateMessageData(gameState.ByteSizeLong() + 1);
        gameStateMessageData[0] = static_cast<uint8_t>(OutgoingPacketType::GameStateUpdate);
        if (const auto serializeGameStateSuccess = gameState.SerializeToArray(
            gameStateMessageData.data() + 1, gameStateMessageData.size() - 1); !serializeGameStateSuccess)
        {
            // TODO -> log error!
            return;
        }
        auto gameStateMessageBuffer = std::make_shared<std::vector<uint8_t>>(std::move(gameStateMessageData));

        ws.binary(true);
        net::post(ws.get_executor(),
                      [self = shared_from_this(), gameStateMessageBuffer]
                      {
                          self->outgoingGameStatePackets.emplace_back(gameStateMessageBuffer);
                          if (!self->writeInProgress)
                          {
                              self->writeGameStatePacket();
                          }

                      }
        );
    }

    void Session::writeGameStatePacket()
    {
        writeInProgress = true;

        const auto gameStatePacketBuffer = outgoingGameStatePackets.front();
        ws.async_write(
            boost::asio::buffer(*gameStatePacketBuffer),
            [self = shared_from_this(), gameStatePacketBuffer](beast::error_code ec, std::size_t bytesTransferred)
            {
                boost::ignore_unused(bytesTransferred);

                if (ec)
                {
                    // TODO -> log!
                    self->writeInProgress = false;
                    return;
                }

                self->outgoingGameStatePackets.pop_front();

                if (self->outgoingGameStatePackets.empty())
                {
                    self->writeInProgress = false;
                }
                else
                {
                    self->writeGameStatePacket();
                }
            }
        );

    }


    void Session::asyncRunHandler()
    {
        // use suggested timeout settings
        ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

        ws.async_accept(
            beast::bind_front_handler(&Session::asyncWebsocketAcceptHandler, shared_from_this())
        );
    }


    /**
     * This is where the session actually "starts", since the websocket connection has been established between
     * client and server.
     * @param ec beast error code
     */
    void Session::asyncWebsocketAcceptHandler(beast::error_code ec)
    {
        if (ec)
        {
            // TODO -> log!
            return;
        }

        sendWpmPacket(WpmPacketType::LOOP_SOUND, static_cast<int32_t>(SoundType::GHOST_ALARM));

        listenToClient();
    }

    void Session::listenToClient()
    {
        ws.async_read(
            incomingClientMessageBuffer,
            beast::bind_front_handler(
                &Session::asyncListenToClientHandler,
                shared_from_this()
            )
        );
    }

    void Session::asyncListenToClientHandler(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec == websocket::error::closed)
        {
            // TODO -> log connection closed
            return;
        }
        if (ec)
        {
            // TODO -> log other error
        }

        // here we should now actually grab the (binary) message;
        if (ws.got_text())
        {
            handleTextMessage();
        }
        else if (ws.got_binary())
        {
            handleBinaryMessage();
        }

        listenToClient();
    }

    void Session::handleTextMessage()
    {
        // these don't do anything atow
        incomingClientMessageBuffer.consume(incomingClientMessageBuffer.size());
    }

    bool Session::isOppositeDirection(Direction direction1, Direction direction2)
    {
        switch (direction1)
        {
        case Direction::UP: return direction2 == Direction::DOWN;
        case Direction::DOWN: return direction2 == Direction::UP;
        case Direction::LEFT: return direction2 == Direction::RIGHT;
        case Direction::RIGHT: return direction2 == Direction::LEFT;
        default: return false;
        }
    }

    void Session::handleBinaryMessage()
    {
        const auto incomingData = incomingClientMessageBuffer.data();

        auto inputMessage = UserInputMessage();
        inputMessage.ParseFromArray(incomingData.data(), static_cast<int32_t>(incomingData.size()));
        //TODO -> input sanitization

        const int32_t incomingPacketType = inputMessage.incomingpackettype();
        int32_t inputDirection = inputMessage.direction();

        switch (incomingPacketType)
        {
        case static_cast<int32_t>(IncomingPacketType::UserInputPress):
            {
                if (game->currentGameMode == GameMode::ATTRACT || game->currentGameMode == GameMode::GAMEPLAY)
                    game->lastBufferedInput = static_cast<Direction>(inputDirection);

                if (game->player.targetCell && isOppositeDirection(game->lastBufferedInput, game->player.orientation))
                {
                    game->player.reverseDirection();
                }

                break;
            }
        case static_cast<int32_t>(IncomingPacketType::UserInputRelease): // NOLINT
            {
                // TODO -> maybe not needed
                break;
            }
        case static_cast<int32_t>(IncomingPacketType::GameModeComplete):
            {
                //TODO
                break;
            }
        default: /* TODO -> log!*/ ;
        }

        incomingClientMessageBuffer.consume(incomingClientMessageBuffer.size());
    }

    void Session::sendWpmPacket(WpmPacketType packetType, int32_t payload)
    {
        auto packet_message = WpmPacket();

        packet_message.set_packettype(static_cast<int32_t>(packetType));
        packet_message.set_payload(payload);

        std::vector<uint8_t> packetData(packet_message.ByteSizeLong() + 1);
        packetData[0] = static_cast<uint8_t>(OutgoingPacketType::WpmPacket);
        if (const auto serializeSoundPacketResult = packet_message.SerializeToArray(
            packetData.data() + 1, packet_message.ByteSizeLong()); !serializeSoundPacketResult)
        {
            // TODO -> log error
            return;
        }
        auto packetBuffer = std::make_shared<std::vector<uint8_t>>(std::move(packetData));

        // new: wpm packets have to use an outgoing packet queue or else we get soft_mutex throws from beast
        ws.binary(true);
        net::post(
            ws.get_executor(),
            [self = shared_from_this(), packetBuffer]
            {
                self->outgoingWpmPackets.emplace_back(std::move(packetBuffer));
                if (!self->writeInProgress)
                {
                    self->writeWpmPacket();
                }

            }
        );
    }

    void Session::writeWpmPacket()
    {
        writeInProgress = true;

        const auto packetBuffer = outgoingWpmPackets.front();

        ws.async_write(
                    boost::asio::buffer(*packetBuffer),
                    // capture the packet data shared pointer so lifetime is guaranteed to end after write finishes
                    [self = shared_from_this(), packetBuffer](beast::error_code ec, std::size_t bytesTransferred)
                    {
                        boost::ignore_unused(bytesTransferred);
                        if (ec)
                        {
                            // TODO -> log
                            self->writeInProgress = false;
                            return;
                        }

                        self->outgoingWpmPackets.pop_front();

                        if (self->outgoingWpmPackets.empty())
                        {
                            self->writeInProgress = false;
                        }
                        else
                        {
                            self->writeWpmPacket();
                        }
                    }
                );
    }


} // pacman
