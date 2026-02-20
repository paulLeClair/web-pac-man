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
            if (const auto scatterCellOrNull = scatterIfNecessary())
            {
                return getClosestNeighborToTargetCell(scatterCellOrNull);
            }

            if (pacmanIsMoreThanEightTilesAway())
            {
                return getClosestNeighborToTargetCell(player->currentCell);
            }
            return getClosestNeighborToTargetCell(clydeScatterCell);
        }

        Clyde() = default;
        ~Clyde() override = default;

    protected:
        MazeCell* getScatterCell() override
        {
            if (!mazeFile) return nullptr;
            return mazeFile->getCell(0, 27);
        }

    private:
        bool pacmanIsMoreThanEightTilesAway() const
        {
            return std::abs(currentCell->gridX - player->currentCell->gridX) >= 8
                || std::abs(currentCell->gridY - player->currentCell->gridY) >= 8;
        }
};

} // pacman
