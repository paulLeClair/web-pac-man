//
// Created by paull on 2026-01-16.
//

#pragma once

#include <cstdint>
#include <vector>

// hopefully this file won't get too unwieldy but I'll just try and shove all the game entity defs in here
enum class CurrentGameState
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
    uint32_t x, y;
};

/**
 * Velocities in terms of screen pixels, using same coordinate system as positions
 */
struct Velocity
{
    uint32_t x, y;
};

struct Entity
{
    Position pos;
    Velocity vel;

    uint32_t currentGridCellX, currentGridCellY;
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
