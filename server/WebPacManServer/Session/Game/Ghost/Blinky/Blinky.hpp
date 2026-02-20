//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Ghost.hpp"

namespace pacman {

struct Blinky final : Ghost {
    Blinky() = default;
    ~Blinky() override = default;

    MazeCell* obtainNextTarget() override
    {
         if (const auto scatterCellOrNull = scatterIfNecessary())
        {
            return getClosestNeighborToTargetCell(scatterCellOrNull);
        }

        return getClosestNeighborToTargetCell(player->currentCell);
    }


protected:
    MazeCell* getScatterCell() override
    {
        if (!mazeFile)
        {
            return nullptr;
        }

        return mazeFile->getCell(0, 0);
    }


};

} // pacman
