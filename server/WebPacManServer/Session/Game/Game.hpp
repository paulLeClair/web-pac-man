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
                                                     inkyStartCell(maze->getCell(25, 28)),
                                                     clydeStartCell(maze->getCell(0, 28)),
                                                     blinkyStartCell(maze->getCell(0, 0))
        {
            startNextLevel();
            currentGameMode = GameMode::ATTRACT;
        }
        std::unique_ptr<MazeFile> maze;

        uint32_t score = 0;

        GameMode currentGameMode = GameMode::UNKNOWN;
        uint8_t level = 1;

        // map of packed grid coordinates to the type of item stored there
        std::unordered_map<uint32_t, ItemType> items;

        uint8_t numberOfLives = 2;

        bool ghostsAreScattering = false;
        uint32_t scatterCountdown = 0;

        Pacman player;
        bool pacmanIsDead = false;
        bool hideBoard = false;
        bool displayReadyMessage = false;
        bool displayAttractMessage = false;
        bool displayGameOverMessage = false;

        Direction lastBufferedInput = Direction::NONE;

        Blinky blinky;
        Pinky pinky;
        Inky inky;
        Clyde clyde;

        // trigger a particular sound to play once all the way through
        static void triggerSound(SoundType soundType, Session& session);
        // trigger a particular sound to loop until stopped
        static void loopSound(SoundType soundType, Session& session);
        // cut a sound short if its currently playing, else nothing will happen
        static void stopSound(SoundType soundType, Session& session);

        void tickGameOver(Session& session);

        void tickIntermission(int32_t intermissionNumber, Session& session);

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
                    tickIntermission(1, session);
                    //todo
                    break;
                }
            case GameMode::INTERMISSION_2:
                {
                    tickIntermission(2, session);
                    //todo
                    break;
                }
            case GameMode::INTERMISSION_3:
                {
                    tickIntermission(3, session);
                    break;
                }
            case GameMode::ATTRACT:
                {
                    tickAttract();
                    break;
                }
            case GameMode::GAME_OVER:
                {
                    tickGameOver(session);
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

        void startNextLevel(const bool resetItems = true)
        {
            if (resetItems) initializeItems();

            preparePlayerForNextLevel();

            prepareGhostForNextLevel(&blinky, blinkyStartCell);

            prepareGhostForNextLevel(&inky, inkyStartCell);

            prepareGhostForNextLevel(&pinky, pinkyStartCell);

            prepareGhostForNextLevel(&clyde, clydeStartCell);
            clyde.clydeScatterCell = clydeStartCell;
        }

        void prepareGhostForNextLevel(Ghost *ghost, MazeCell *startCell)
        {
            // TODO -> start ghosts in center and only allow them to leave after a set period of time
            ghost->setNormalSpeed();
            ghost->player = &player;
            ghost->mazeFile = maze.get();
            ghost->currentCell = startCell;
            ghost->isDead = false;
            ghost->isScattering = false;
            ghost->hidden = false;
            ghost->inJail = false;
            ghost->targetCell = nullptr;
        }

        void preparePlayerForNextLevel()
        {
            static constexpr float DEFAULT_PLAYER_SPEED = 0.15;

            pacmanIsDead = false;
            player.isChomping = true;
            player.currentCell = pacmanStartCell;
            player.orientation = Direction::LEFT;
            player.bufferedInput = Direction::LEFT;
            player.speed = DEFAULT_PLAYER_SPEED;
            player.mazeFile = maze.get();
            player.hidden = false;

            player.targetCell = player.obtainNextTarget();
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
                        ghost.isDead = true;
                        ghost.setFastSpeed();
                        ghost.inJail = false;
                    }
                }
                else
                {
                    stopAllSounds(session);
                    currentGameMode = GameMode::FAILURE;
                }
            }
        }

        // this should be moved to the ghost itself
        [[nodiscard]] bool ghostIsCollidingWithPacman(const Ghost& ghost) const
        {
            if (ghost.isDead) return false;
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

        void stopScattering(Session& session);
        void startScattering(Session& session, const int& scatterTimeout);

        static void stopAllSounds(Session& session)
        {
            sendSoundPacket(SoundType::GHOST_ALARM, WpmPacketType::STOP_SOUND, session);
            sendSoundPacket(SoundType::GHOSTS_SCATTERING, WpmPacketType::STOP_SOUND, session);
            sendSoundPacket(SoundType::PACMAN_EATING, WpmPacketType::STOP_SOUND, session);
            sendSoundPacket(SoundType::SPECIAL_ITEM_GET, WpmPacketType::STOP_SOUND, session);
        }

        static void sendSoundPacket(SoundType soundType, WpmPacketType soundControlType, Session& session);
        static void sendCutsceneTriggerPacket(int32_t cutsceneNumber, Session& session);

        void hideCharacters();
        void showCharacters();

        void tickStart(Session& session);

        void tickGameplay(Session& session);

        void tickSuccess(Session& session);

        void tickFailure(Session& session);

        void tickAttract();
    };
} // pacman
