//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../../Defines.hpp"

enum class GhostName
{
    INKY,
    PINKY,
    BLINKY,
    CLYDE
};

enum class GhostOrientation : int32_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Ghost : Entity
{
    GhostName ghostName = GhostName::CLYDE;

    bool isDead = false;

    GhostOrientation orientation = GhostOrientation::UP;

};
