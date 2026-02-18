//
// Created by paull on 2026-01-20.
//

#include "Player.hpp"

namespace pacman
{
    MazeCell* Pacman::getNextWalkableCellByOrientation()
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
                        bufferedInput = Direction::NONE;
                        return result;
                    }
                case Direction::UP:
                    {
                        const auto result = getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY - 1);
                        if (result != nullptr) orientation = Direction::UP;
                        bufferedInput = Direction::NONE;
                        return result;
                    }
                case Direction::LEFT:
                    {
                        const auto result = getMazeCellIfWalkable(currentCell->gridX - 1, currentCell->gridY);
                        if (result != nullptr) orientation = Direction::LEFT;
                        bufferedInput = Direction::NONE;
                        return result;
                    }
                case Direction::RIGHT:
                    {
                        const auto result = getMazeCellIfWalkable(currentCell->gridX + 1, currentCell->gridY);
                        if (result != nullptr) orientation = Direction::RIGHT;
                        bufferedInput = Direction::NONE;
                        return result;
                    }
                case Direction::NONE:
                    {
                        return getNextWalkableCellByOrientation();
                    }
                default: return nullptr;
                }
            }
        case TileType::OPEN:
            {
                return getNextWalkableCellByOrientation();
            }
        default: return nullptr;
        }
    }

    void Pacman::update()
    {
        // set the target cell if we're stationary at an intersection
        if (!targetCell || mazeFile->getTileType(currentCell->gridX, currentCell->gridY) == TileType::INTERSECTION)
        {
            switch (bufferedInput)
            {
            case Direction::DOWN:
                {
                    targetCell = getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY + 1);
                    if (targetCell != nullptr)
                    {
                        orientation = Direction::DOWN;
                        isChomping = true;
                    }

                    bufferedInput = Direction::NONE;
                    break;
                }
            case Direction::UP:
                {
                    targetCell = getMazeCellIfWalkable(currentCell->gridX, currentCell->gridY - 1);
                    if (targetCell != nullptr)
                    {
                        orientation = Direction::UP;
                        isChomping = true;
                    }

                    bufferedInput = Direction::NONE;
                    break;
                }
            case Direction::LEFT:
                {
                    targetCell = getMazeCellIfWalkable(currentCell->gridX - 1, currentCell->gridY);
                    if (targetCell != nullptr)
                    {
                        orientation = Direction::LEFT;
                        isChomping = true;
                    }

                    bufferedInput = Direction::NONE;
                    break;
                }
            case Direction::RIGHT:
                {
                    targetCell = getMazeCellIfWalkable(currentCell->gridX + 1, currentCell->gridY);
                    if (targetCell != nullptr)
                    {
                        orientation = Direction::RIGHT;
                        isChomping = true;
                    }

                    bufferedInput = Direction::NONE;
                    break;
                }
            default: break;
            }
        }

        // TODO -> allow for inputs to instantly 180 pacman if its directed opposite his orientation (into a valid tile)

        this->Entity::update();

        if (!targetCell)
        {
            isChomping = false;
        }
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
