//
// Created by paull on 2026-01-15.
//

#include "Session.hpp"

#include "../../protobuf/gen/game_state.pb.h"
#include "../../protobuf/gen/user_inputs.pb.h"

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
        game->tick(*this); // new idea: pass session into game tick so we can shoot other stuff before the state update

        // for now naive flat update of all state variables at once
        GameStateMessage gameState;

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

        gameStateMessageData.resize(gameState.ByteSizeLong() + 1);
        gameStateMessageData[0] = static_cast<uint8_t>(OutgoingPacketType::GameStateUpdate);
        if (const auto serializeGameStateSuccess = gameState.SerializeToArray(
            gameStateMessageData.data() + 1, gameStateMessageData.size() - 1); !serializeGameStateSuccess)
        {
            // TODO -> log error!
            return;
        }

        const auto mutableBuffer = gameStateFlatBuffer.prepare(gameStateMessageData.size());
        boost::asio::buffer_copy(mutableBuffer, boost::asio::buffer(gameStateMessageData));
        gameStateFlatBuffer.commit(gameStateMessageData.size());

        ws.binary(true);
        ws.async_write(
            gameStateFlatBuffer.data(),
            beast::bind_front_handler(&Session::asyncWriteGameStateHandler, shared_from_this())
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

        // TODO -> here we would first run the intro cutscene but that can come after sound is confirmed working

        // when we begin gameplay, we'll also want to have the ghosts spawn in the jail and come out in the proper order
        game->loopSound(SoundType::GHOST_ALARM, *this);

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

    /**
     * This is the async callback for initial testing where we're just echoing back;
     * not sure if this will really factor in to the core functionality of the session
     * @param ec boost error code
     * @param bytes_transferred
     */
    void Session::asyncTestEchoInputHandler(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
        {
            // TODO -> log error!
            return;
        }

        // this clears the buffer;
        incomingClientMessageBuffer.consume(incomingClientMessageBuffer.size());

        listenToClient();
    }

    void Session::asyncWriteGameStateHandler(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
        {
            // TODO -> log error!
            return;
        }

        // clear gamestate buffer (does this need sync?)
        gameStateFlatBuffer.consume(gameStateFlatBuffer.size());

        // after we write game state, we can just drop the async call
    }

    void Session::handleTextMessage()
    {
        // these don't do much atow
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
} // pacman
