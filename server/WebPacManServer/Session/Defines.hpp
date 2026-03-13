//
// Created by paull on 2026-01-16.
//

#pragma once
#include "Game/Maze/Maze.hpp"

enum class ItemType : uint32_t
{
    UNKNOWN = 0,
    DOT = 10,
    ENERGIZER = 50,
    CHERRY = 100,
    STRAWBERRY = 300,
    ORANGE = 500,
    APPLE = 700,
    MELON = 1000,
    GALAXIAN = 2000,
    BELL = 3000,
    KEY = 5000
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
    NONE = 0,
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4,
};

enum class SoundType : int32_t
{
    UNKNOWN = 0,
    INTRO_THEME,
    PACMAN_EATING,
    GHOST_ALARM,
    GHOSTS_SCATTERING,
    GHOST_EATEN,
};

enum class WpmPacketType
{
    UNKNOWN = 0,
    TRIGGER_SOUND,
    LOOP_SOUND,
    STOP_SOUND
};

enum class GameMode
{
    UNKNOWN = 0,
    START,
    GAMEPLAY,
    SUCCESS,
    FAILURE,
    INTERMISSION_1,
    INTERMISSION_2,
};


enum class IncomingPacketType : int32_t
{
    UserInputPress = 0x101,
    UserInputRelease = 0x102, // maybe unused
    GameModeComplete = 0x103,
};

enum class OutgoingPacketType : uint8_t
{
    GameStateUpdate = 0x70,
    SoundControlPacket = 0x80,
};

struct Entity
{
    virtual ~Entity() = default;
    Position pos = {0,0};
    Direction orientation = Direction::NONE;

    // allows hiding the entity (for cutscenes etc)
    bool hidden = false;

    // how fast the interpolation parameter changes each tick
    float speed = 0.08f;

    // current interpolation parameter value between currentcell and targetcell (if one exists)
    float param = 0.0f;

    // last cell that the entity either departed from or is still sitting on (depending if targetCell != nullptr)
    MazeCell *currentCell = nullptr;

    // either the next cell that the entity is moving towards each tick or null if the entity is stationary
    MazeCell *targetCell = nullptr;

    virtual MazeCell* obtainNextTarget()
    {
        return nullptr;
    }

    virtual void update()
    {
        if (!targetCell) return;

        param = std::min(param + speed, 1.0f);

        const float pixelDeltaX = param * static_cast<float>(targetCell->gridX - currentCell->gridX) * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS;
        const float pixelDeltaY = param * static_cast<float>(targetCell->gridY - currentCell->gridY) * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS;

        pos.x = static_cast<float>(currentCell->gridX) * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS + pixelDeltaX;
        pos.y = static_cast<float>(currentCell->gridY) * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS + pixelDeltaY;

        if (param == 1.0f)
        {
            currentCell = targetCell;
            param = 0.0f;
            targetCell = obtainNextTarget();
        }
    }


};