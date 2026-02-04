//
// Created by paull on 2026-01-20.
//

#pragma once

#include <unordered_set>

#include "../Defines.hpp"
#include "Ghost/Blinky/Blinky.hpp"
#include "Ghost/Clyde/Clyde.hpp"
#include "Ghost/Inky/Inky.hpp"
#include "Ghost/Pinky/Pinky.hpp"
#include "Player/Player.hpp"

namespace pacman {

class Game {
public:
    CurrentGameMode currentGameState = CurrentGameMode::START_SCREEN;
    Items items;
    MapGrid mapGrid;

    bool ghostsAreScattering = false;

    Pacman player;

    // NOTE: i'm fairly certain boost asio should sync this, but we may need a lock
    InputDirection lastBufferedInput = {};

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
