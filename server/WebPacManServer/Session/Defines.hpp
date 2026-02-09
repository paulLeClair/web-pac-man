//
// Created by paull on 2026-01-16.
//

#pragma once
#include "Game/Maze/Maze.hpp"

// hopefully this file won't get too unwieldy but I'll just try and shove all the game entity defs in here
enum class CurrentGameMode
{
    START_SCREEN, // "attract" mode which can be interrupted to initiate gameplay
    GAMEPLAY,
    INTERMISSION_1, // level 2
    INTERMISSION_2, // level 5
    INTERMISSION_3 // level 9
};

/**
 * "World-space" positions, with the bottom left of the screen as the origin
 */
struct Position
{
    float x, y;
};

enum class Direction : int32_t
{
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

struct Entity
{
    virtual ~Entity() = default;
    Position pos = {0,0};
    Direction orientation = Direction::NONE;

    // for now lets try a simple static speed
    const float speed = 0.08f;

    float param = 0.0f;
    MazeCell *currentCell = nullptr;
    MazeCell *targetCell = nullptr;

    virtual MazeCell* obtainNextTarget()
    {
        return nullptr;
    }

    virtual void update()
    {
        if (!targetCell) return;

        param += std::min(speed, 1.0f);

        float deltaX = 0.0f, deltaY = 0.0f;

        switch (orientation)
        {
            case Direction::UP: deltaY = -1 * param * (targetCell->pixelY - currentCell->pixelY); break;
            case Direction::DOWN: deltaY = param * (targetCell->pixelY - currentCell->pixelY); break;
            case Direction::LEFT: deltaX = -1 * param * (targetCell->pixelX - currentCell->pixelX); break;
            case Direction::RIGHT: deltaX = param * (targetCell->pixelX - currentCell->pixelX); break;
            default: break;
        }

        pos.x = currentCell->pixelX + deltaX;
        pos.y = currentCell->pixelY + deltaY;

        if (param == 1.0f)
        {
            currentCell = targetCell;

            targetCell = obtainNextTarget();
        }
    }
};

struct Items
{

};