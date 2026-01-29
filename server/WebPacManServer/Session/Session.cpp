//
// Created by paull on 2026-01-15.
//

#include "Session.hpp"

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

            // ws.async_write(
            //     gameStateBuffer.data(),
            //     beast::bind_front_handler(&Session::asyncWriteGameStateHandler, shared_from_this())
            // );
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

        readUserInputs();
    }

    void Session::readUserInputs()
    {
        ws.async_read(
            userInputBuffer,
            beast::bind_front_handler(
                &Session::asyncReadUserInputHandler,
                shared_from_this()
            )
        );
    }

    void Session::asyncReadUserInputHandler(beast::error_code ec, std::size_t bytes_transferred)
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

        // here we would now actually grab the (binary) message;
        // for testing purposes i should probably just have it echo text first
        ws.text(ws.got_text());
        ws.async_write(
            userInputBuffer.data(),
            beast::bind_front_handler(&Session::asyncTestEchoInputHandler, shared_from_this())
        );
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
        userInputBuffer.consume(userInputBuffer.size());

        readUserInputs();
    }

    void Session::asyncWriteGameStateHandler(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
        {
            // TODO -> log error!
            return;
        }
    }
} // pacman
