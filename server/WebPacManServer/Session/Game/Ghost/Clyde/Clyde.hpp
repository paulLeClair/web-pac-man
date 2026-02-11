//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Ghost.hpp"

namespace pacman {

    struct Clyde final : Ghost {
        MazeCell *clydeScatterCell = nullptr;

        MazeCell* obtainNextTarget() override
        {
            if (pacmanIsMoreThanEightTilesAway())
            {
                return getClosestNeighborToTargetCell(player->currentCell);
            }
            return getClosestNeighborToTargetCell(clydeScatterCell);
        }

        Clyde() = default;
        ~Clyde() override = default;

    private:
        bool pacmanIsMoreThanEightTilesAway() const
        {
            return std::abs(currentCell->gridX - player->currentCell->gridX) >= 8
                || std::abs(currentCell->gridY - player->currentCell->gridY) >= 8;
        }
};

} // pacman
