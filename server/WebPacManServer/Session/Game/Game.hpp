//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Defines.hpp"
#include "Ghost/Blinky/Blinky.hpp"
#include "Ghost/Clyde/Clyde.hpp"
#include "Ghost/Inky/Inky.hpp"
#include "Ghost/Pinky/Pinky.hpp"
#include "Player/Player.hpp"

namespace pacman {

class Game {
public:

    // game state
    CurrentGameState currentGameState = CurrentGameState::START_SCREEN;
    Items items;
    MapGrid mapGrid;

    Pacman player;

    Blinky blinky;
    Pinky pinky;
    Inky inky;
    Clyde clyde;

    void tick()
    {

    }
};

} // pacman
