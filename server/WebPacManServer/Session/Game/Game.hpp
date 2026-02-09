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
    explicit Game(const std::string &fileName) : maze(std::make_unique<MazeFile>(fileName))
    {
        // testing/debug
        auto startPos = maze->getCell(21, 19);

        // init player etc
        player.mazeFile = maze.get();
        player.isChomping = false;
        player.pos = {startPos->pixelX, startPos->pixelY}; // todo -> player starting position
        player.currentCell = startPos;
        player.orientation = Direction::LEFT;
        player.bufferedInput = Direction::NONE;

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

};

} // pacman
