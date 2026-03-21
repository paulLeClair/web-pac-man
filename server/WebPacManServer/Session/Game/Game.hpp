//
// Created by paull on 2026-01-20.
//

#pragma once

#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>

#include "../Defines.hpp"
#include "Ghost/Blinky/Blinky.hpp"
#include "Ghost/Clyde/Clyde.hpp"
#include "Ghost/Inky/Inky.hpp"
#include "Ghost/Pinky/Pinky.hpp"
#include "Maze/Maze.hpp"
#include "Player/Player.hpp"

namespace pacman
{
    class Session;

    class Game : public std::enable_shared_from_this<Game>
    {
    public:
        explicit Game(const std::string& fileName) : maze(std::make_unique<MazeFile>(fileName)),
                                                     pacmanStartCell(maze->getCell(13, 22)),
                                                     pinkyStartCell(maze->getCell(25, 1)),
                                                     blinkyStartCell(maze->getCell(0, 0)),
                                                     inkyStartCell(maze->getCell(25, 28)),
                                                     clydeStartCell(maze->getCell(0, 28))
        {
            setupGameEntities();
            currentGameMode = GameMode::START;
        }

        uint32_t score = 0;

        GameMode currentGameMode = GameMode::GAMEPLAY;
        uint8_t level = 1;

        // map of packed grid coordinates to the type of item stored there
        std::unordered_map<uint32_t, ItemType> items;

        std::unique_ptr<MazeFile> maze;

        bool ghostsAreScattering = false;
        uint32_t scatterCountdown = 0;

        Pacman player;

        // NOTE: i'm fairly certain boost asio should sync this, but we may need a lock
        Direction lastBufferedInput = {};

        Blinky blinky;
        Pinky pinky;
        Inky inky;
        Clyde clyde;

        bool displayReadyMessage;

        // trigger a particular sound to play once all the way through
        void triggerSound(SoundType soundType, Session& session);
        // trigger a particular sound to loop until stopped
        void loopSound(SoundType soundType, Session& session);
        // cut a sound short if its currently playing, else nothing will happen
        void stopSound(SoundType soundType, Session& session);

        void tick(Session& session)
        {
            switch (currentGameMode)
            {
            case GameMode::START:
                {
                    tickStart(session);
                    break;
                }
            case GameMode::GAMEPLAY:
                {
                    tickGameplay(session);
                    break;
                }
            case GameMode::SUCCESS:
                {
                    tickSuccess(session);
                    break;
                }
            case GameMode::FAILURE:
                {
                    tickFailure(session);
                    break;
                }
            case GameMode::INTERMISSION_1:
                {
                    //todo
                    break;
                }
            case GameMode::INTERMISSION_2:
                {
                    //todo
                    break;
                }
            default:
                {

                };
            }
        }

    private:
        MazeCell* pacmanStartCell;
        MazeCell* pinkyStartCell;
        MazeCell* inkyStartCell;
        MazeCell* clydeStartCell;
        MazeCell* blinkyStartCell;

        // TODO -> move private function defs to cpp file

        // temporary hacky "reset" function
        void setupGameEntities()
        {
            static constexpr float DEFAULT_PLAYER_SPEED = 0.15;
            static constexpr float DEFAULT_GHOST_SPEED = 0.15;

            player.isChomping = false;
            player.currentCell = pacmanStartCell;
            player.targetCell = nullptr;
            player.orientation = Direction::LEFT;
            player.bufferedInput = Direction::NONE;
            player.speed = DEFAULT_PLAYER_SPEED;

            player.mazeFile = maze.get();

            pinky.player = &player;
            pinky.mazeFile = maze.get();
            pinky.currentCell = pinkyStartCell;
            pinky.targetCell = nullptr;
            pinky.isDead = false;
            player.speed = DEFAULT_GHOST_SPEED;

            inky.player = &player;
            inky.mazeFile = maze.get();
            inky.currentCell = inkyStartCell;
            inky.targetCell = nullptr;
            inky.isDead = false;
            inky.speed = DEFAULT_GHOST_SPEED;

            blinky.player = &player;
            blinky.mazeFile = maze.get();
            blinky.currentCell = blinkyStartCell;
            blinky.targetCell = nullptr;
            blinky.isDead = false;
            blinky.speed = DEFAULT_GHOST_SPEED;

            clyde.player = &player;
            clyde.mazeFile = maze.get();
            clyde.currentCell = clydeStartCell;
            clyde.clydeScatterCell = clydeStartCell;
            clyde.targetCell = nullptr;
            clyde.isDead = false;
            clyde.speed = DEFAULT_GHOST_SPEED;

            initializeItems();
        }

        static bool gridCellIsBlacklisted(const MazeCell& mazeCell)
        {
            static const std::unordered_set<uint32_t> blacklistedLocations = {
                13,
                1 << 16 | 13,
                2 << 16 | 13,
                3 << 16 | 13,
                4 << 16 | 13,
                21 << 16 | 13,
                22 << 16 | 13,
                23 << 16 | 13,
                24 << 16 | 13,
                25 << 16 | 13,
                12 << 16 | 22,
                13 << 16 | 22,
            };
            return blacklistedLocations.contains(mazeCell.gridX << 16 | mazeCell.gridY);
        }

        boost::beast::flat_buffer outgoingSoundMessageBuffer = boost::beast::flat_buffer();

        void initializeItems()
        {
            static const std::unordered_set<uint32_t> energizerLocations = {
                2,
                25 << 16 | 2,
                22,
                25 << 16 | 22
            };

            for (auto& walkableCell : maze->getAllValidCells())
            {
                if (gridCellIsBlacklisted(walkableCell))
                {
                    continue;
                }

                if (walkableCell.gridX <= 5 || walkableCell.gridX >= 20
                    || walkableCell.gridY <= 7 || walkableCell.gridY >= 19)
                {
                    uint32_t packedCoords = walkableCell.gridX << 16 | walkableCell.gridY;
                    if (energizerLocations.contains(packedCoords))
                    {
                        items.insert({packedCoords, ItemType::ENERGIZER});
                        continue;
                    }
                    items.insert({packedCoords, ItemType::DOT});
                }
            }
        }

        void update_ghost_state(Ghost& ghost, Session& session)
        {
            ghost.update();
            if (ghostIsCollidingWithPacman(ghost))
            {
                if (ghostsAreScattering)
                {
                    if (!ghost.isDead)
                    {
                        // TODO -> increase speed and set target cell as entrance to the ghost jail
                        ghost.isDead = true;
                        ghost.speed *= 2;
                        ghost.inJail = false;
                    }
                }
                else
                {
                    // TODO -> overhaul how game over is handled/detected
                    // setupGameEntities();
                    // stopAllSounds(session);
                    // testing:
                    currentGameMode = GameMode::START;
                }
            }
        }

        // this should be moved to the ghost itself
        [[nodiscard]] bool ghostIsCollidingWithPacman(const Ghost& ghost) const
        {
            float currentPlayerPosX = player.currentCell->gridX;
            float currentPlayerPosY = player.currentCell->gridY;
            if (player.targetCell)
            {
                currentPlayerPosX
                    = player.currentCell->gridX + player.param * (player.targetCell->gridX - player.currentCell->gridX); //NOLINT
                currentPlayerPosY
                    = player.currentCell->gridY + player.param * (player.targetCell->gridY - player.currentCell->gridY); //NOLINT
            }
            float currentGhostPosX = ghost.currentCell->gridX;
            float currentGhostPosY = ghost.currentCell->gridY;
            if (ghost.targetCell)
            {
                currentGhostPosX
                    = ghost.currentCell->gridX + ghost.param * (ghost.targetCell->gridX - ghost.currentCell->gridX); //NOLINT
                currentGhostPosY
                    = ghost.currentCell->gridY + ghost.param * (ghost.targetCell->gridY - ghost.currentCell->gridY); //NOLINT
            }

            static constexpr float epsilon = 1;

            return std::abs(currentGhostPosX - currentPlayerPosX) < epsilon && std::abs(
                currentGhostPosY - currentPlayerPosY) < epsilon;
        }

        void stopAllSounds(Session& session)
        {
            // stopSound(SoundType::GHOST_ALARM, session);
            // stopSound(SoundType::PACMAN_EATING, session);
        }

        // send a websocket message for the client to trigger a sound on the next update
        void sendSoundPacket(SoundType soundType, WpmPacketType soundControlType, Session& session);

        void asyncSoundPacketWriteHandler(boost::beast::error_code ec, std::size_t bytesTransferred);

        void tickStart(Session& session);
        void stopScattering(Session& session);

        void tickGameplay(Session& session);

        void tickSuccess(Session& session);

        void tickFailure(Session& session);
    };
} // pacman
