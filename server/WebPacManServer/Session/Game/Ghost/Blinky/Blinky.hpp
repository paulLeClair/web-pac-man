//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Ghost.hpp"

namespace pacman {

struct Blinky final : Ghost {
    Blinky();

    explicit Blinky(MazeFile *mazeFile) : Ghost(mazeFile) {};

    ~Blinky() override = default;

    MazeCell* obtainNextTarget() override
    {
        // new: all ghosts need to be checking if they're in jail and going there if necessary
        if (isDead && !inJail)
        {
            return goToJail();
        }
        if (inJail)
        {
            return bounceInJailUntilRespawn();
        }

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
