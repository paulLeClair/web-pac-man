//
// Created by paull on 2026-01-16.
//

#pragma once

#include <cstdint>
#include <vector>

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

enum InputDirection
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Entity
{
    virtual ~Entity() = default;
    Position pos = {0,0};

    virtual void update()
    {

    }
};

// TODO -> formalize a minimalistic and efficient map data structure
struct MapGridCell
{
    uint8_t x, y;

    MapGridCell *leftNeighbor;
    MapGridCell *rightNeighbor;
    MapGridCell *upNeighbor;
    MapGridCell *downNeighbor;
};

/**
 * For simplicity, the actual world is just going to be modeled as a map grid, where
 * only the accessible grid cells are considered
 */
struct MapGrid
{
    uint16_t width, height;

    uint8_t tileWidthInPixels;




};

/**
 * This contains a breakdown of all the current items on the screen
 */
struct Items
{
    std::vector<MapGridCell*> pellets;
    std::vector<MapGridCell*> powerPellets;
    std::vector<MapGridCell*> cherries;
    // etc etc

};
