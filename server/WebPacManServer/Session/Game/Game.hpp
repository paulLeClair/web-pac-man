//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Defines.hpp"
#include "Ghost/Blinky/Blinky.hpp"
#include "Ghost/Clyde/Clyde.hpp"
#include "Ghost/Inky/Inky.hpp"
#include "Ghost/Pinky/Pinky.hpp"
#include "Maze/Maze.hpp"
#include "Player/Player.hpp"

namespace pacman {


class Game {
public:

    explicit Game(const std::string &fileName) : maze(std::make_unique<MazeFile>(fileName)),
                                                 pacmanStartCell(maze->getCell(13, 22)),
                                                 pinkyStartCell(maze->getCell(25, 1)),
                                                 blinkyStartCell(maze->getCell(0,0)),
                                                 inkyStartCell(maze->getCell(25, 28)),
                                                 clydeStartCell(maze->getCell(0, 28))
    {
        setupGameEntities();
    }

    uint32_t score = 0;

    CurrentGameMode currentGameState = CurrentGameMode::GAMEPLAY;

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

    void tick()
    {
        static constexpr auto scatterTimeout = 180;

        player.bufferedInput = lastBufferedInput;
        player.update();

        if (ghostsAreScattering)
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
            }
        }

        if (const uint32_t packedPlayerCoords = player.currentCell->gridX << 16 | player.currentCell->gridY;
            items.contains(packedPlayerCoords))
        {
            auto obtainedItemType = items.at(packedPlayerCoords);
            score += static_cast<uint32_t>(obtainedItemType); // item type enum holds score directly

            if (obtainedItemType == ItemType::ENERGIZER)
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
            }

            if (!items.erase(packedPlayerCoords))
            {
                // TODO -> log error...
                return;
            }

            if (items.empty())
            {
                // this would trigger the next stage
                setupGameEntities();
            }
        }
        // TODO -> display score


        // todo-> make a general "update_ghost_state()" instead to reduce code duplication
        update_pinky_state();
        update_blinky_state();
        update_inky_state();
        update_clyde_state();

        // TODO -> timer/score etc
    }

    MazeCell *pacmanStartCell;
    MazeCell* pinkyStartCell;
    MazeCell* inkyStartCell;
    MazeCell* clydeStartCell;
    MazeCell* blinkyStartCell;


private:
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

    static bool gridCellIsBlacklisted(const MazeCell &mazeCell)
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

        for (auto &walkableCell : maze->getAllValidCells())
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

    void update_pinky_state()
    {
        pinky.update();
        if (ghostIsCollidingWithPacman(pinky))
        {
            if (ghostsAreScattering)
            {
                if (!pinky.isDead)
                {
                    // TODO -> increase speed and set target cell as entrance to the ghost jail
                    pinky.isDead = true;
                    pinky.speed *= 2;
                    pinky.inJail = false;
                }
            }
            else
            {
                // restart the game i guess
                setupGameEntities();
            }
        }
    }

    void update_blinky_state()
    {
        blinky.update();
        if (ghostIsCollidingWithPacman(blinky))
        {
            if (ghostsAreScattering)
            {
                if (!blinky.isDead)
                {
                    // TODO -> increase speed and set target cell as entrance to the ghost jail
                    blinky.isDead = true;
                    blinky.speed *= 2;
                    blinky.inJail = false;
                }
            }
            else
            {
                // restart the game i guess
                setupGameEntities();
            }
        }
    }

    void update_inky_state()
    {
        inky.update();
        if (ghostIsCollidingWithPacman(inky))
        {
            if (ghostsAreScattering)
            {
                if (!inky.isDead)
                {
                    // TODO -> increase speed and set target cell as entrance to the ghost jail
                    inky.isDead = true;
                    inky.speed *= 2;
                    inky.inJail = false;
                }
            }
            else
            {
                // restart the game i guess
                setupGameEntities();
            }
        }
    }

    void update_clyde_state()
    {
        clyde.update();
        if (ghostIsCollidingWithPacman(clyde))
        {
            if (ghostsAreScattering)
            {
                if (!clyde.isDead)
                {
                    // TODO -> increase speed and set target cell as entrance to the ghost jail
                    clyde.isDead = true;
                    clyde.speed *= 2;
                    clyde.inJail = false;
                }
            }
            else if (!clyde.isDead)
            {
                // restart the game i guess
                setupGameEntities();
            }
        }
    }

    [[nodiscard]] bool pinkyIsCollidingWithPacman() const
    {
        return pinky.currentCell == player.currentCell;
    }

    [[nodiscard]] bool blinkyIsCollidingWithPacman() const
    {
        return blinky.currentCell == player.currentCell;
    }

    [[nodiscard]] bool inkyIsCollidingWithPacman() const
    {
        return inky.currentCell == player.currentCell;
    }

    [[nodiscard]] bool clydeIsCollidingWithPacman() const
    {
        return clyde.currentCell == player.currentCell;
    }

    bool ghostIsCollidingWithPacman(const Ghost &ghost) const
    {
        float currentPlayerPosX = player.currentCell->gridX;
        float currentPlayerPosY = player.currentCell->gridY;
        if (player.targetCell)
        {
            currentPlayerPosX  = player.currentCell->gridX + player.param * (player.targetCell->gridX - player.currentCell->gridX);
            currentPlayerPosY = player.currentCell->gridY + player.param * (player.targetCell->gridY - player.currentCell->gridY);
        }
        float currentGhostPosX = ghost.currentCell->gridX;
        float currentGhostPosY = ghost.currentCell->gridY;
        if (ghost.targetCell)
        {
            currentGhostPosX = ghost.currentCell->gridX + ghost.param * (ghost.targetCell->gridX - ghost.currentCell->gridX);
            currentGhostPosY = ghost.currentCell->gridY + ghost.param * (ghost.targetCell->gridY - ghost.currentCell->gridY);
        }

        static constexpr float epsilon = 1;

        return std::abs(currentGhostPosX - currentPlayerPosX) < epsilon && std::abs(currentGhostPosY - currentPlayerPosY) < epsilon;
    }
};

} // pacman
