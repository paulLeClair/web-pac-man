//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../../Defines.hpp"
#include "../Maze/Maze.hpp"
#include "../Player/Player.hpp"

#include <ranges>

enum class GhostName
{
    INKY,
    PINKY,
    BLINKY,
    CLYDE
};

enum class GhostOrientation : int32_t
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct Ghost : Entity
{
    explicit Ghost(MazeFile *mazeFile) : mazeFile(mazeFile) {}

    MazeFile *mazeFile = nullptr;

    GhostName ghostName = GhostName::CLYDE;

    // note: theres a nomenclature mismatch between this and OG pacman: scattering is frightened, wander is scattering.
    // also: if not wandering and not scattering, we can assume the ghost is chasing.
    bool isWandering = false;

    // note: theres a nomenclature mismatch between this and OG pacman: scattering is frightened, wander is scattering
    bool isScattering = false;

    // this signals the walk back to jail if the ghost has been eaten by pacman
    bool isDead = false;

    // signals whether the ghost is currently confined to the ghost jail
    bool inJail = false;

    pacman::Pacman *player = nullptr;

    void update() override
    {
        if (!mazeFile || !player) return;

        if (!previousCell)
        {
            previousCell = currentCell;
        }

        // todo -> wire in being dead/scattered once items are added
        if (!targetCell)
        {
            targetCell = obtainNextTarget();
        }

        const auto *preUpdateCurrentCell = currentCell;
        Entity::update();
        if (currentCell != preUpdateCurrentCell)
        {
            previousCell = currentCell;
        }


    }

    MazeCell *obtainNextTarget() override
    {
        // base ghost impl: nothing
        return nullptr;
    }

    void setNormalSpeed()
    {
        static constexpr float DEFAULT_NORMAL_SPEED = 0.14;
        speed = DEFAULT_NORMAL_SPEED;
    }

    void setSlowSpeed()
    {
        static constexpr float DEFAULT_SLOW_SPEED = 0.10;
        speed = DEFAULT_SLOW_SPEED;
    }

    void setFastSpeed()
    {
        static constexpr float DEFAULT_FAST_SPEED = 0.16;
        speed = DEFAULT_FAST_SPEED;
    }
protected:
    MazeCell *previousCell = nullptr;
    int jailTimer = 0;

    static float getCellToCellDistance(const MazeCell *cell1, const MazeCell *cell2)
    {
        if (!cell1 || !cell2) return NAN;
        return sqrt(pow( (cell2->gridX - cell1->gridX), 2)
            + pow((cell2->gridY - cell1->gridY), 2));
    }

    virtual MazeCell *getScatterCell() = 0;

    MazeCell *getClosestNeighborToTargetCell(const MazeCell *potentiallyUnwalkableTargetCell)
    {
        std::unordered_map<Direction, float> directionDistances;

        const auto *rightNeighbor = mazeFile->getCell(currentCell->gridX + 1, currentCell->gridY);
        const auto *leftNeighbor = mazeFile->getCell(currentCell->gridX - 1, currentCell->gridY);
        const auto *upNeighbor = mazeFile->getCell(currentCell->gridX, currentCell->gridY - 1);
        const auto *downNeighbor = mazeFile->getCell(currentCell->gridX, currentCell->gridY + 1);

        if (mazeFile->isWalkable(currentCell->gridX + 1, currentCell->gridY) && rightNeighbor != previousCell)
        {
            const float rightNeighborDistance = getCellToCellDistance(
                potentiallyUnwalkableTargetCell,
                mazeFile->getCell(currentCell->gridX + 1, currentCell->gridY));
            directionDistances[Direction::RIGHT] = rightNeighborDistance;
        }
        if (mazeFile->isWalkable(currentCell->gridX - 1, currentCell->gridY) && leftNeighbor != previousCell)
        {
            const float leftNeighborDistance = getCellToCellDistance(
                potentiallyUnwalkableTargetCell,
                mazeFile->getCell(currentCell->gridX - 1, currentCell->gridY));
            directionDistances[Direction::LEFT] = leftNeighborDistance;
        }
        if (mazeFile->isWalkable(currentCell->gridX, currentCell->gridY + 1) && downNeighbor != previousCell)
        {
            const float downNeighborDistance = getCellToCellDistance(
                potentiallyUnwalkableTargetCell,
                mazeFile->getCell(currentCell->gridX, currentCell->gridY + 1)
            );
            directionDistances[Direction::DOWN] = downNeighborDistance;
        }
        if (mazeFile->isWalkable(currentCell->gridX, currentCell->gridY - 1) && upNeighbor != previousCell)
        {
            const float upNeighborDistance = getCellToCellDistance(
                potentiallyUnwalkableTargetCell,
                mazeFile->getCell(currentCell->gridX, currentCell->gridY - 1)
            );
            directionDistances[Direction::UP] = upNeighborDistance;
        }

        float minTargetDistance = 1e20;
        for (auto& dist : directionDistances | std::views::values)
        {
            minTargetDistance = std::min(minTargetDistance, dist);
        }

        auto minimumDirection = Direction::NONE;
        for (auto &[dir, dist] : directionDistances)
        {
            if (dist == minTargetDistance) minimumDirection = dir;
        }

        switch (minimumDirection)
        {
            case Direction::UP:
                {
                    orientation = Direction::UP;
                    return mazeFile->getCell(currentCell->gridX, currentCell->gridY - 1);
                }
            case Direction::DOWN:
                {
                    orientation = Direction::DOWN;
                    return mazeFile->getCell(currentCell->gridX, currentCell->gridY + 1);
                }
            case Direction::LEFT:
                {
                    orientation = Direction::LEFT;
                    return mazeFile->getCell(currentCell->gridX - 1, currentCell->gridY);
                }
            case Direction::RIGHT:
                {
                    orientation = Direction::RIGHT;
                    return mazeFile->getCell(currentCell->gridX + 1, currentCell->gridY);
                }
            default: return nullptr;
        }
    }


    MazeCell *scatterIfNecessary()
    {
        if (!isScattering) return nullptr;
        return getScatterCell();
    }

    MazeCell* obtainDefeatedGhostTarget()
    {
        if (isDead && !inJail)
        {
            return goToJail();
        }
        if (inJail)
        {
            return bounceInJailUntilRespawn();
        }
        return nullptr; // log
    }


    MazeCell* bounceInJailUntilRespawn()
    {
        if (jailTimer == 0)
        {
            isDead = false;
            inJail = false;
            return const_cast<MazeCell*>(mazeFile->getGhostJailEntryCell());
        }

        // here we would just switch between 2 bounce points, probably also just coming from the maze itself
        jailTimer--;

        if (currentCell == MazeFile::getGhostJailLeftBounceCell())
        {
            return const_cast<MazeCell*>(MazeFile::getGhostJailRightBounceCell());
        }
        if (currentCell == MazeFile::getGhostJailRightBounceCell())
        {
            return const_cast<MazeCell*>(MazeFile::getGhostJailLeftBounceCell());
        }
        return const_cast<MazeCell*>(MazeFile::getGhostJailLeftBounceCell());
    }

    MazeCell *goToJail()
    {
        if (const auto jail_entry_cell = mazeFile->getGhostJailEntryCell(); currentCell != jail_entry_cell)
        {
            return getClosestNeighborToTargetCell(jail_entry_cell);
        }

        // here we would enter the jail
        jailTimer = 20;
        inJail = true;
        setSlowSpeed();
        return bounceInJailUntilRespawn();
    }
};
