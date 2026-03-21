//
// Created by paull on 2026-03-06.
//
#include "Game.hpp"
#include "../Session.hpp"
#include "../../../protobuf/gen/wpm_packet.pb.h"

namespace pacman
{
    void Game::triggerSound(const SoundType soundType, Session& session)
    {
        sendSoundPacket(soundType, WpmPacketType::TRIGGER_SOUND, session);
    }

    void Game::loopSound(const SoundType soundType, Session& session)
    {
        sendSoundPacket(soundType, WpmPacketType::LOOP_SOUND, session);
    }

    void Game::stopSound(const SoundType soundType, Session& session)
    {
        sendSoundPacket(soundType, WpmPacketType::STOP_SOUND, session);
    }

    void Game::sendSoundPacket(SoundType soundType, WpmPacketType soundControlType, Session& session)
    {
        // very simple, just pack up the appropriate flags
        auto triggerSoundPacketMessage = WpmPacket();

        triggerSoundPacketMessage.set_packettype(static_cast<int32_t>(soundControlType));
        triggerSoundPacketMessage.set_payload(static_cast<int32_t>(soundType));

        std::vector<uint8_t> packetData(triggerSoundPacketMessage.ByteSizeLong() + 1);
        packetData[0] = static_cast<uint8_t>(OutgoingPacketType::SoundControlPacket);
        if (const auto serializeSoundPacketResult = triggerSoundPacketMessage.SerializeToArray(
            packetData.data() + 1, triggerSoundPacketMessage.ByteSizeLong()); !serializeSoundPacketResult)
        {
            // log error
            return;
        }

        outgoingSoundMessageBuffer.consume(packetData.size()); // clear any previous data
        const auto mutableBuffer = outgoingSoundMessageBuffer.prepare(packetData.size());
        boost::asio::buffer_copy(mutableBuffer, boost::asio::buffer(packetData));
        outgoingSoundMessageBuffer.commit(packetData.size());

        session.ws.binary(true);

        boost::asio::dispatch(
            session.ws.get_executor(),
            [&]
            {
                session.ws.async_write(
                    outgoingSoundMessageBuffer.data(),
                    beast::bind_front_handler(&Game::asyncSoundPacketWriteHandler, shared_from_this())
                );
            }
        );
    }

    void Game::asyncSoundPacketWriteHandler(beast::error_code ec, std::size_t bytesTransferred)
    {
        boost::ignore_unused(bytesTransferred);

        if (ec)
        {
            // log!
        }
    }

    // this should play the intro theme with all entities hidden and display the "READY!" message; once the song is over,
    // transition straight into gameplay mode
    void Game::tickStart(Session& session)
    {
        using namespace std::chrono_literals;
        static boost::optional<std::chrono::time_point<std::chrono::steady_clock>> countdownStartPoint = boost::none;

        // hide characters if not already hidden
        if (!player.hidden) player.hidden = true;
        if (!clyde.hidden) clyde.hidden = true;
        if (!inky.hidden) inky.hidden = true;
        if (!blinky.hidden) blinky.hidden = true;
        if (!pinky.hidden) pinky.hidden = true;

        if (!countdownStartPoint.has_value())
        {
            // first we would trigger the sound
            triggerSound(SoundType::INTRO_THEME, session);
            countdownStartPoint = std::chrono::steady_clock::now();
        }

        // display ready message
        displayReadyMessage = true;

        auto currentTime = std::chrono::steady_clock::now();
        if (countdownStartPoint.has_value() && currentTime - *countdownStartPoint >= 5s)
        {
            displayReadyMessage = false;
            player.hidden = false;
            clyde.hidden = false;
            inky.hidden = false;
            blinky.hidden = false;
            pinky.hidden = false;

            // we need to be jumping into the failure animation here and then either display "game over" or
            // subtract a life and respawn
            currentGameMode = GameMode::GAMEPLAY;

            countdownStartPoint = boost::none;
            setupGameEntities();
        }
    }

    void Game::stopScattering(Session& session)
    {
        scatterCountdown--;
        if (scatterCountdown == 0)
        {
            blinky.isScattering = false;
            blinky.setNormalSpeed();

            pinky.isScattering = false;
            pinky.setNormalSpeed();

            inky.isScattering = false;
            inky.setNormalSpeed();

            clyde.isScattering = false;
            clyde.setNormalSpeed();

            ghostsAreScattering = false;
            // stopSound(SoundType::GHOSTS_SCATTERING, session);
            // loopSound(SoundType::GHOST_ALARM, session);
        }
    }

    void Game::tickGameplay(Session& session)
    {
        static constexpr auto scatterTimeout = 180; // TODO -> tweak this to match original game

        player.bufferedInput = lastBufferedInput;
        player.update();

        if (ghostsAreScattering)
        {
            stopScattering(session);
        }

        if (const uint32_t packedPlayerCoords = player.currentCell->gridX << 16 | player.currentCell->gridY;
            items.contains(packedPlayerCoords))
        {
            auto obtainedItemType = items.at(packedPlayerCoords);
            score += static_cast<uint32_t>(obtainedItemType); // item type enum holds score directly

            switch (obtainedItemType)
            {
            case ItemType::ENERGIZER:
                {
                    // activate scattering effect
                    ghostsAreScattering = true;

                    blinky.isScattering = true;
                    blinky.setSlowSpeed();

                    pinky.isScattering = true;
                    pinky.setSlowSpeed();

                    inky.isScattering = true;
                    inky.setSlowSpeed();

                    clyde.isScattering = true;
                    clyde.setSlowSpeed();

                    scatterCountdown = scatterTimeout;
                    // stopSound(SoundType::GHOST_ALARM, session);
                    // loopSound(SoundType::GHOSTS_SCATTERING, session);
                    break;
                }
            case ItemType::DOT:
                {
                    // TODO -> we probably want to be starting/stopping a loop with the waka-waka sound
                }
            default: ;
            }

            if (!items.erase(packedPlayerCoords))
            {
                // TODO -> log error...
                return;
            }

            if (items.empty())
            {
                // this would trigger the SUCCESS game mode
                setupGameEntities();
            }
        }

        update_ghost_state(pinky, session);
        update_ghost_state(blinky, session);
        update_ghost_state(inky, session);
        update_ghost_state(clyde, session);

    }

    void Game::tickSuccess(Session& session)
    {
        // display level transition message; (future: display intermissions at appropriate level milestones)
    }

    void Game::tickFailure(Session& session)
    {
        // hide all items and freeze movement, trigger failure sound, (future: pacman dying animation)
    }
}
