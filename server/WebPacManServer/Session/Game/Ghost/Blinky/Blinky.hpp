//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Ghost.hpp"

namespace pacman {

struct Blinky final : Ghost {
    Blinky() : Ghost(nullptr)
    {
    }

    explicit Blinky(MazeFile *mazeFile) : Ghost(mazeFile) {};

    ~Blinky() override = default;

    MazeCell* obtainNextTarget() override
    {
        if (isDead || inJail) return obtainDefeatedGhostTarget();

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
