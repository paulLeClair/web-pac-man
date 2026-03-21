//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Ghost.hpp"

namespace pacman
{
    struct Clyde final : Ghost
    {
        Clyde() : Ghost(nullptr)
        {
        }

        explicit Clyde(MazeFile* maze) : Ghost(maze)
        {
        }

        MazeCell* clydeScatterCell = nullptr;

        MazeCell* obtainNextTarget() override
        {
            if (isDead || inJail) return obtainDefeatedGhostTarget();

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

        ~Clyde() override = default;

    protected:
        MazeCell* getScatterCell() override
        {
            if (!mazeFile) return nullptr;
            return mazeFile->getCell(0, 27);
        }

    private:
        [[nodiscard]] bool pacmanIsMoreThanEightTilesAway() const
        {
            return std::abs(currentCell->gridX - player->currentCell->gridX) >= 8
                || std::abs(currentCell->gridY - player->currentCell->gridY) >= 8;
        }
    };
} // pacman
