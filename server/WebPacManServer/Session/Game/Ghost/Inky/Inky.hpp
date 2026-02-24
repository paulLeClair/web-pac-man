//
// Created by paull on 2026-01-20.
//

#pragma once

#include <ranges>

#include "../Ghost.hpp"

namespace pacman {

struct Inky final : Ghost {
    Inky() : Ghost(nullptr) {};
    explicit Inky(MazeFile *maze) : Ghost(maze) {}
    ~Inky() override = default;

    // note, this is most likely not good to plug in as the target cell; we should use it to choose a walkable cell
    MazeCell potentiallyUnwalkableTargetCell;

    // inky's is the weirdest...
    MazeCell *obtainNextTarget() override
    {
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

        // TODO -> inky draws a line two tiles ahead of pacman and doubles it
        MazeCell pacmanOffsetCell;
        switch (player->orientation)
        {
            case Direction::UP:
                {
                    pacmanOffsetCell = MazeCell(
                        player->currentCell->gridX,
                        player->currentCell->gridY - 2,
                        TileType::OPEN
                    );

                    break;
                }
            case Direction::DOWN:
                {
                    pacmanOffsetCell = MazeCell(
                        player->currentCell->gridX,
                        player->currentCell->gridY + 2,
                        TileType::OPEN
                    );
                    break;
                }
            case Direction::RIGHT:
                {
                    pacmanOffsetCell = MazeCell(
                        player->currentCell->gridX + 2,
                        player->currentCell->gridY,
                        TileType::OPEN
                    );
                    break;
                }
            case Direction::LEFT:
                {
                    pacmanOffsetCell = MazeCell(
                        player->currentCell->gridX - 2,
                        player->currentCell->gridY,
                        TileType::OPEN
                    );
                    break;
                }
            default:
                {
                    pacmanOffsetCell = *player->currentCell;
                }
        }

        // not sure if this is what the pacman dossier means but it seems reasonable
        potentiallyUnwalkableTargetCell = MazeCell(
            currentCell->gridX + 2 * (pacmanOffsetCell.gridX - currentCell->gridX),
            currentCell->gridY + 2 * (pacmanOffsetCell.gridY - currentCell->gridY),
            TileType::OPEN
        );

        return getClosestNeighborToTargetCell(&potentiallyUnwalkableTargetCell);
    }

protected:
    MazeCell* getScatterCell() override
    {
        if (!mazeFile) return nullptr;
        return mazeFile->getCell(24, 27); // verify this
    }
};

} // pacman
