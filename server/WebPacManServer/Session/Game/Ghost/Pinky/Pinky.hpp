//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Ghost.hpp"

namespace pacman {

struct Pinky final : Ghost {
    // note, this is most likely not good to plug in as the target cell; we should use it to choose a walkable cell
    MazeCell potentiallyUnwalkableTargetCell;

    Pinky() = default;
    ~Pinky() override = default;

    MazeCell *obtainNextTarget() override
    {
        if (const auto scatterCellOrNull = scatterIfNecessary())
        {
            return getClosestNeighborToTargetCell(scatterCellOrNull);
        }

        // pinky targeting logic
        switch (player->orientation)
        {
            case Direction::UP:
                {
                    potentiallyUnwalkableTargetCell = MazeCell(
                        player->currentCell->gridX,
                        player->currentCell->gridY - 4,
                        TileType::OPEN,
                        0,
                        0
                    );
                } break;
            case Direction::DOWN:
                {
                    potentiallyUnwalkableTargetCell = MazeCell(
                        player->currentCell->gridX,
                        player->currentCell->gridY + 4,
                        TileType::OPEN,
                        0,
                        0
                    );
                } break;
            case Direction::LEFT:
                {
                    potentiallyUnwalkableTargetCell = MazeCell(
                        player->currentCell->gridX - 4,
                        player->currentCell->gridY,
                        TileType::OPEN,
                        0,
                        0
                    );


                } break;
            case Direction::RIGHT:
                {
                    potentiallyUnwalkableTargetCell = MazeCell(
                        player->currentCell->gridX + 4,
                        player->currentCell->gridY,
                        TileType::OPEN,
                        0,
                        0
                    );
                } break;
            default: return nullptr;
        }

        return getClosestNeighborToTargetCell(&potentiallyUnwalkableTargetCell);
    }

protected:
    MazeCell* getScatterCell() override
    {
        if (!mazeFile) return nullptr;

        return mazeFile->getCell(25, 1);
    }
};

} // pacman
