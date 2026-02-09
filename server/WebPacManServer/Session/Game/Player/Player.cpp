//
// Created by paull on 2026-01-20.
//

#include "Player.hpp"

namespace pacman
{
    MazeCell* Pacman::obtainNextTarget()
    {
        if (!mazeFile)
        {
            // TODO -> log error!
            return nullptr;
        }

        switch (mazeFile->getTileType(currentCell->gridX, currentCell->gridY))
        {
        case TileType::INTERSECTION:
            {
                switch (bufferedInput)
                {
                case Direction::DOWN:
                    {
                        const auto result = getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY + 1);
                        if (result != nullptr) orientation = Direction::DOWN;
                        return result;
                    }
                case Direction::UP:
                    {
                        const auto result = getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY - 1);
                        if (result != nullptr) orientation = Direction::UP;
                        return result;
                    }
                case Direction::LEFT:
                    {
                        const auto result = getMazeCellIfWalkable(currentCell->gridX - 1, currentCell->gridY);
                        if (result != nullptr) orientation = Direction::LEFT;
                        return result;
                    }
                case Direction::RIGHT:
                    {
                        const auto result = getMazeCellIfWalkable(currentCell->gridX + 1, currentCell->gridY);
                        if (result != nullptr) orientation = Direction::RIGHT;
                        return result;
                    }
                default: return nullptr;
                }
            }
        case TileType::OPEN:
            {
                switch (orientation)
                {
                case Direction::DOWN:
                    {
                        return getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY + 1);
                    }
                case Direction::UP:
                    {
                        return getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY - 1);
                    }
                case Direction::LEFT:
                    {
                        return getMazeCellIfWalkable(currentCell->gridX - 1, currentCell->gridY);
                    }
                case Direction::RIGHT:
                    {
                        return getMazeCellIfWalkable(currentCell->gridX + 1, currentCell->gridY);
                    }
                default: return nullptr;
                }
            }
        default: return nullptr;
        }
    }

    void Pacman::update()
    {
        // set the target cell if we're stationary at an intersection

        if (!targetCell && mazeFile->getTileType(currentCell->gridX, currentCell->gridY) == TileType::INTERSECTION)
        {
            switch (bufferedInput)
            {
            case Direction::DOWN:
                {
                    targetCell = getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY + 1);
                    if (targetCell != nullptr) orientation = Direction::DOWN;
                }
            case Direction::UP:
                {
                    targetCell = getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY - 1);
                    if (targetCell != nullptr) orientation = Direction::UP;
                }
            case Direction::LEFT:
                {
                    targetCell = getMazeCellIfWalkable(currentCell->gridX - 1, currentCell->gridY);
                    if (targetCell != nullptr) orientation = Direction::LEFT;
                }
            case Direction::RIGHT:
                {
                    targetCell = getMazeCellIfWalkable(currentCell->gridX + 1, currentCell->gridY);
                    if (targetCell != nullptr) orientation = Direction::RIGHT;
                }
            default: break;
            }
        }

        this->Entity::update();
    }

    MazeCell* Pacman::getMazeCellIfWalkable(const int x, const int y) const
    {
        if (mazeFile->isWalkable(x, y))
        {
            return mazeFile->getCell(x, y);
        }
        return nullptr;
    }
} // pacman
