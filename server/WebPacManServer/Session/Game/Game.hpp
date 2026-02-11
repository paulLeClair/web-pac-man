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
        pacmanStartCell(maze->getCell(14, 23)),
        pinkyStartCell(maze->getCell(25, 1)),
        blinkyStartCell(maze->getCell(0,0)),
        inkyStartCell(maze->getCell(25, 28)),
        clydeStartCell(maze->getCell(0, 28))
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

        inky.player = &player;
        inky.mazeFile = maze.get();
        inky.currentCell = inkyStartCell;

        blinky.player = &player;
        blinky.mazeFile = maze.get();
        blinky.currentCell = blinkyStartCell;

        clyde.player = &player;
        clyde.mazeFile = maze.get();
        clyde.currentCell = clydeStartCell;
        clyde.clydeScatterCell = clydeStartCell;
    }

    CurrentGameMode currentGameState = CurrentGameMode::START_SCREEN;
    Items items;

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

        blinky.update();
        pinky.update();
        inky.update();
        clyde.update();

        // TODO -> timer/score etc
    }

private:
    MazeCell *pacmanStartCell;
    MazeCell* pinkyStartCell;
    MazeCell* inkyStartCell;
    MazeCell* clydeStartCell;
    MazeCell* blinkyStartCell;
};

} // pacman
