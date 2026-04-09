//
// Created by paull on 2026-03-06.
//
#include "Game.hpp"
#include "../Session.hpp"

namespace pacman
{
    using namespace std::chrono_literals;

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

    void Game::sendSoundPacket(SoundType soundType, const WpmPacketType soundControlType, Session& session)
    {
        session.sendWpmPacket(soundControlType, static_cast<int32_t>(soundType));
    }

    void Game::sendCutsceneTriggerPacket(const int32_t cutsceneNumber, Session& session)
    {
        session.sendWpmPacket(WpmPacketType::TRIGGER_CUTSCENE, cutsceneNumber);
    }

    // this should definitely get moved to session


    void Game::hideCharacters()
    {
        if (!player.hidden) player.hidden = true;
        if (!clyde.hidden) clyde.hidden = true;
        if (!inky.hidden) inky.hidden = true;
        if (!blinky.hidden) blinky.hidden = true;
        if (!pinky.hidden) pinky.hidden = true;
    }

    void Game::showCharacters()
    {
        player.hidden = false;
        clyde.hidden = false;
        inky.hidden = false;
        blinky.hidden = false;
        pinky.hidden = false;
    }
    // this should play the intro theme with all entities hidden and display the "READY!" message; once the song is over,
    // transition straight into gameplay mode
    void Game::tickStart(Session& session)
    {
        static boost::optional<std::chrono::time_point<std::chrono::steady_clock>> countdownStartPoint = boost::none;


        if (!countdownStartPoint.has_value())
        {
            // first we would trigger the sound
            triggerSound(SoundType::INTRO_THEME, session);
            countdownStartPoint = std::chrono::steady_clock::now();
            // hide characters if not already hidden
            hideCharacters();
            // display ready message
            displayReadyMessage = true;
        }


        if (const auto currentTime = std::chrono::steady_clock::now();
            countdownStartPoint.has_value() && currentTime - *countdownStartPoint >= 5s)
        {
            displayReadyMessage = false;
            showCharacters();

            // we need to be jumping into the failure animation here and then either display "game over" or
            // subtract a life and respawn
            currentGameMode = GameMode::GAMEPLAY;
            startNextLevel();
            loopSound(SoundType::GHOST_ALARM, session);

            countdownStartPoint = boost::none;
        }
    }

    void Game::stopScattering(Session& session)
    {
        scatterCountdown--;
        if (scatterCountdown == 0)
        {
            if (!blinky.isDead)
            {
                blinky.isScattering = false;
                blinky.setNormalSpeed();
            }

            if (!pinky.isDead)
            {
                pinky.isScattering = false;
                pinky.setNormalSpeed();
            }

            if (!inky.isDead)
            {
                inky.isScattering = false;
                inky.setNormalSpeed();
            }

            if (!clyde.isDead)
            {
                clyde.isScattering = false;
                clyde.setNormalSpeed();
            }

            ghostsAreScattering = false;
            stopSound(SoundType::GHOSTS_SCATTERING, session);
            loopSound(SoundType::GHOST_ALARM, session);
        }
    }

    void Game::startScattering(Session& session, const int& scatterTimeout)
    {
        // activate scattering effect
        ghostsAreScattering = true;

        if (!blinky.isDead)
        {
            blinky.isScattering = true;
            blinky.setSlowSpeed();
        }

        if (!pinky.isDead)
        {
            pinky.isScattering = true;
            pinky.setSlowSpeed();
        }

        if (!inky.isDead)
        {
            inky.isScattering = true;
            inky.setSlowSpeed();
        }

        if (!clyde.isDead)
        {
            clyde.isScattering = true;
            clyde.setSlowSpeed();
        }

        scatterCountdown = scatterTimeout;
        stopSound(SoundType::GHOST_ALARM, session);
        loopSound(SoundType::GHOSTS_SCATTERING, session);
    }

    void Game::tickGameplay(Session& session)
    {
        static constexpr auto scatterTimeout = 180; // TODO -> tweak this to match original game
        static bool wakaWaka = false;

        player.bufferedInput = lastBufferedInput;
        player.update();

        if (ghostsAreScattering)
        {
            stopScattering(session);
        }

        // this check needs to be redone a little bit;
        if (const uint32_t packedPlayerCoords = player.currentCell->gridX << 16 | player.currentCell->gridY;
            items.contains(packedPlayerCoords))
        {
            auto obtainedItemType = items.at(packedPlayerCoords);
            score += static_cast<uint32_t>(obtainedItemType); // item type enum holds score directly

            switch (obtainedItemType)
            {
            case ItemType::ENERGIZER:
                {
                    startScattering(session, scatterTimeout);
                    break;
                }
            case ItemType::DOT:
                {
                    // TODO -> we probably want to be starting/stopping a loop with the waka-waka sound
                    if (!wakaWaka)
                    {
                        wakaWaka = true;
                        loopSound(SoundType::PACMAN_EATING, session);
                    }
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
                currentGameMode = GameMode::SUCCESS;
            }
        }
        else
        {
            // stop looping the chomp sound
            if (wakaWaka &&
                (!player.targetCell || !items.contains(player.targetCell->gridX << 16 | player.targetCell->gridY)))
            {
                wakaWaka = false;
                stopSound(SoundType::PACMAN_EATING, session);
            }
        }

        update_ghost_state(pinky, session);
        update_ghost_state(blinky, session);
        update_ghost_state(inky, session);
        update_ghost_state(clyde, session);
    }

    void Game::tickSuccess(Session& session)
    {
        static boost::optional<std::chrono::time_point<std::chrono::steady_clock>> countdownStart = boost::none;

        hideCharacters();

        if (!countdownStart.has_value())
        {
            countdownStart = std::chrono::steady_clock::now();
            stopAllSounds(session);
            // probably also want to add a little "success!" message that can be toggled on and off
            // or play a little jingle to signal that the level was completed
        }

        if (const auto currentTime = std::chrono::steady_clock::now();
            countdownStart.has_value() && currentTime - *countdownStart >= 4s)
        {
            // here we need to increment the level and play any intermission animations that apply
            level++;
            if (level == 2)
            {
                currentGameMode = GameMode::INTERMISSION_1;
                return;
            }
            if (level == 5)
            {
                currentGameMode = GameMode::INTERMISSION_2;
                return;
            }
            if (level == 9 || level % 4 == 0)
            {
                currentGameMode = GameMode::INTERMISSION_3;
                return;
            }

            currentGameMode = GameMode::GAMEPLAY;
            loopSound(SoundType::GHOST_ALARM, session);
        }
    }

    void Game::tickFailure(Session& session)
    {
        static boost::optional<std::chrono::time_point<std::chrono::steady_clock>> countdownStart = boost::none;

        if (!countdownStart.has_value())
        {
            stopAllSounds(session);
            triggerSound(SoundType::GAME_OVER, session);
            countdownStart = std::chrono::steady_clock::now();
            // hide all items and freeze movement, trigger failure sound, trigger dying animation
            hideCharacters();
            player.hidden = false;
            pacmanIsDead = true;
            items.clear();
            numberOfLives--;
        }

        if (const auto currentTime = std::chrono::steady_clock::now();
            countdownStart.has_value() && currentTime - *countdownStart >= 2s)
        {
            player.hidden = true;
        }

            if (const auto currentTime = std::chrono::steady_clock::now();
            countdownStart.has_value() && currentTime - *countdownStart >= 3.75s)
        {
            countdownStart = boost::none;

            // if we have any lives left, we just jump back into gameplay
            if (numberOfLives)
            {
                currentGameMode = GameMode::GAMEPLAY;
                startNextLevel();
                loopSound(SoundType::GHOST_ALARM, session);
            }
            else
            {
                currentGameMode = GameMode::GAME_OVER;
            }
        }
    }

    void Game::tickAttract()
    {
        hideCharacters();
        items.clear();
        hideBoard = false;

        // todo -> probably just have a randomly-controlled player running the game loop with no sounds,
        // for now just blank screen with "press any button to start" message that needs to be added to client assets

        // this should just wait for a user input
        if (lastBufferedInput != Direction::NONE)
        {
            lastBufferedInput = Direction::NONE;
            currentGameMode = GameMode::START;
        }
    }

    // this should just display a short game over message and then return to attract mode probably
    void Game::tickGameOver(Session& session)
    {
        static boost::optional<std::chrono::time_point<std::chrono::steady_clock>> countdownStart = boost::none;

        hideCharacters();
        hideBoard = true;

        // TODO -> add toggle-able "GAME OVER" message and display it here

        if (!countdownStart.has_value())
        {
            countdownStart = std::chrono::steady_clock::now();
            stopAllSounds(session);
        }

        if (const auto currentTime = std::chrono::steady_clock::now();
            countdownStart.has_value() && currentTime - *countdownStart >= 3s)
        {
            countdownStart = boost::none;
            lastBufferedInput = Direction::NONE;
            currentGameMode = GameMode::ATTRACT;
        }
    }

    void Game::tickIntermission(const int32_t intermissionNumber, Session& session)
    {
        static boost::optional<std::chrono::time_point<std::chrono::steady_clock>> countdownStart = boost::none;

        if (!countdownStart.has_value())
        {
            stopAllSounds(session); // just in case
            triggerSound(SoundType::COFFEE_BREAK, session);
            sendCutsceneTriggerPacket(intermissionNumber, session);
            countdownStart = std::chrono::steady_clock::now();
        }

        if (const auto currentTime = std::chrono::steady_clock::now();
            countdownStart.has_value() && currentTime - *countdownStart >= 14s)
        {
            // cutscene over, transition back to gameplay
            currentGameMode = GameMode::GAMEPLAY;
            loopSound(SoundType::GHOST_ALARM, session);
        }
    }
}
