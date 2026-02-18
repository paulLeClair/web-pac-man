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

    Pacman player;

    // NOTE: i'm fairly certain boost asio should sync this, but we may need a lock
    Direction lastBufferedInput = {};

    Blinky blinky;
    Pinky pinky;
    Inky inky;
    Clyde clyde;


    void tick()
    {
        player.bufferedInput = lastBufferedInput;
        player.update();

        if (items.contains(player.currentCell->gridX << 16 | player.currentCell->gridY))
        {
            score += static_cast<uint32_t>(items.at(player.currentCell->gridX << 16 | player.currentCell->gridY));
        }
        // TODO -> display score

        blinky.update();
        pinky.update();
        inky.update();
        clyde.update();

        if (ghostsAreCollidingWithPacman())
        {
            // restart the game i guess
            setupGameEntities();
        }

        // TODO -> timer/score etc
    }

private:
    MazeCell *pacmanStartCell;
    MazeCell* pinkyStartCell;
    MazeCell* inkyStartCell;
    MazeCell* clydeStartCell;
    MazeCell* blinkyStartCell;


    void setupGameEntities()
    {
        player.isChomping = false;
        player.currentCell = pacmanStartCell;
        player.targetCell = nullptr;
        player.orientation = Direction::LEFT;
        player.bufferedInput = Direction::NONE;

        player.mazeFile = maze.get();

        pinky.player = &player;
        pinky.mazeFile = maze.get();
        pinky.currentCell = pinkyStartCell;
        pinky.targetCell = nullptr;

        inky.player = &player;
        inky.mazeFile = maze.get();
        inky.currentCell = inkyStartCell;
        inky.targetCell = nullptr;

        blinky.player = &player;
        blinky.mazeFile = maze.get();
        blinky.currentCell = blinkyStartCell;
        blinky.targetCell = nullptr;

        clyde.player = &player;
        clyde.mazeFile = maze.get();
        clyde.currentCell = clydeStartCell;
        clyde.clydeScatterCell = clydeStartCell;
        clyde.targetCell = nullptr;

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
            // break; // TESTING
        }

    }

    [[nodiscard]] bool ghostsAreCollidingWithPacman() const
    {
        // TODO -> compare actual current interpolated positions, not just cell
        return player.currentCell == clyde.currentCell
            || player.currentCell == pinky.currentCell
            || player.currentCell == inky.currentCell
            || player.currentCell == blinky.currentCell;
    }
};

} // pacman
