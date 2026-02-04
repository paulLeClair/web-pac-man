//
// Created by paull on 2026-01-15.
//

#include "Session.hpp"

#include "../../protobuf/gen/game_state.pb.h"

namespace pacman
{
    Session::Session(tcp::socket &&socket) : sessionId(boost::uuids::uuid()), ws(std::move(socket))
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
            game.tick();

            // TODO -> BUILD GAME STATE SNAPSHOT AND PUT IT IN GAMESTATEBUFFER
            GameStateMessage gameState;

            // for now naive flat update of all state
            gameState.set_ghostsarescattering(game.ghostsAreScattering);

            // player update
            gameState.set_pacmanischomping(game.player.isChomping);
            gameState.set_pacmanorientation(static_cast<int32_t>(game.player.orientation));
            gameState.set_pacmanpositionx(game.player.pos.x);
            gameState.set_pacmanpositiony(game.player.pos.y);

            // ghost updates
            gameState.set_blinkyisdead(game.blinky.isDead);
            gameState.set_blinkyorientation(static_cast<int32_t>(game.blinky.orientation));
            gameState.set_blinkypositionx(game.blinky.pos.x);
            gameState.set_blinkypositiony(game.blinky.pos.y);

            gameState.set_inkyisdead(game.inky.isDead);
            gameState.set_inkyorientation(static_cast<int32_t>(game.inky.orientation));
            gameState.set_inkypositionx(game.inky.pos.x);
            gameState.set_inkypositiony(game.inky.pos.y);

            gameState.set_pinkyisdead(game.pinky.isDead);
            gameState.set_pinkyorientation(static_cast<int32_t>(game.pinky.orientation));
            gameState.set_pinkypositionx(game.pinky.pos.x);
            gameState.set_pinkypositiony(game.pinky.pos.y);

            gameState.set_clydeisdead(game.clyde.isDead);
            gameState.set_clydeorientation(static_cast<int32_t>(game.clyde.orientation));
            gameState.set_clydepositionx(game.clyde.pos.x);
            gameState.set_clydepositiony(game.clyde.pos.y);

            std::vector<uint8_t> data(gameState.ByteSizeLong());
            if (const auto serializeGameStateSuccess = gameState.SerializeToArray(data.data(), data.size()); !serializeGameStateSuccess)
            {
                // TODO -> log error!
                return;
            }

            gameStateBuffer.clear();
            boost::asio::buffer_copy(gameStateBuffer.data(), boost::asio::buffer(data));

            ws.async_write(
                gameStateBuffer.data(),
                beast::bind_front_handler(&Session::asyncWriteGameStateHandler, shared_from_this())
            );
        }

    void Session::asyncRunHandler()
    {
        // use suggested timeout settings
        ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

        ws.async_accept(
            beast::bind_front_handler(&Session::asyncAcceptHandler, shared_from_this())
        );
    }


    void Session::asyncAcceptHandler(beast::error_code ec)
    {
        if (ec)
        {
            // TODO -> log!
            return;
        }

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

        // clear gamestate buffer
        gameStateBuffer.consume(gameStateBuffer.size());

        // after we write game state, we can just drop the async call
    }

    void Session::handleTextMessage()
    {
        // TODO -> these don't serve any purpose atow
    }

    void Session::handleBinaryMessage()
    {
        const auto incomingData = incomingClientMessageBuffer.data();
        if (incomingData.size() < 8)
        {
            // TODO -> log!
            return;
        }

        // DEBUG/TESTING:
        std::cout << "User Input received: ";
        int32_t incomingPacketType;
        int32_t inputDirection;
        std::cout << "dir=" << inputDirection;

        // the first 4 bytes are dedicated to marking packet type
        switch (incomingPacketType)
        {
        case static_cast<int32_t>(IncomingPacketType::UserInputPress):
            {
                // the next 4 bytes are the direction released
                std::cout << " pressed; ";
                game.heldInputs.insert(static_cast<InputDirection>(inputDirection));
                break;
            }
            case static_cast<int32_t>(IncomingPacketType::UserInputRelease):
            {
                std::cout << " released; ";
                game.heldInputs.erase(static_cast<InputDirection>(inputDirection));
                break;
            }
        case static_cast<int32_t>(IncomingPacketType::GameModeComplete):
            {
                //TODO
                break;
            }
        default: /* TODO -> log!*/ ;
        }

        // DEBUG: print all held inputs
        std::cout << "held inputs: {";
        for (const auto &input : game.heldInputs)
        {
            std::cout << std::to_string(input) << ", ";
        }
        std::cout << "}" << std::endl;

    }
} // pacman
