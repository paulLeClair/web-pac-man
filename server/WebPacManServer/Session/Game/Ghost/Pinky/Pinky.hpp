//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../Ghost.hpp"

namespace pacman {

struct Pinky final : Ghost {
    // note, this is most likely not good to plug in as the target cell; we should use it to choose a walkable cell
    MazeCell potentiallyUnwalkableTargetCell;

    Pinky() : Ghost(nullptr) {}
    explicit Pinky(MazeFile *file) : Ghost(file) {}

    ~Pinky() override = default;


    MazeCell *obtainNextTarget() override
    {
        if (isDead || inJail) return obtainDefeatedGhostTarget();

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
                        TileType::OPEN
                    );
                } break;
            case Direction::DOWN:
                {
                    potentiallyUnwalkableTargetCell = MazeCell(
                        player->currentCell->gridX,
                        player->currentCell->gridY + 4,
                        TileType::OPEN
                    );
                } break;
            case Direction::LEFT:
                {
                    potentiallyUnwalkableTargetCell = MazeCell(
                        player->currentCell->gridX - 4,
                        player->currentCell->gridY,
                        TileType::OPEN
                    );


                } break;
            case Direction::RIGHT:
                {
                    potentiallyUnwalkableTargetCell = MazeCell(
                        player->currentCell->gridX + 4,
                        player->currentCell->gridY,
                        TileType::OPEN
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
