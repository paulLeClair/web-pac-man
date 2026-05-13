#pragma once

#include "../../Defines.hpp"
#include "../Maze/Maze.hpp"
#include "../Player/Player.hpp"

#include <ranges>
#include <boost/optional/optional.hpp>

using namespace std::chrono_literals;

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
    explicit Ghost(MazeFile* mazeFile, MazeCell *ghostJailBounceCell) : mazeFile(mazeFile)
    {
        init_ghost_jail_bounce_cells(ghostJailBounceCell);
    }

    MazeFile* mazeFile = nullptr;

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

    // the number of pacdots that pacman must eat before this ghost can leave the spawn jail
    int jailDotCount = 0;

    // whether the ghost is in jail at initial spawn time
    bool inSpawnJail = false;
    pacman::Pacman* player = nullptr;

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

        const auto* preUpdateCurrentCell = currentCell;
        Entity::update();
        if (currentCell != preUpdateCurrentCell)
        {
            previousCell = currentCell;
        }
    }

    void decrementDotCount()
    {
        if (!inSpawnJail) return;
        jailDotCount--;
    }

    MazeCell* obtainNextTarget() override
    {
        // base ghost impl: nothing
        return nullptr;
    }

    void setNormalSpeed()
    {
        static constexpr float DEFAULT_NORMAL_SPEED = 0.13;
        speed = DEFAULT_NORMAL_SPEED;
    }

    void setSlowSpeed()
    {
        static constexpr float DEFAULT_SLOW_SPEED = 0.07;
        speed = DEFAULT_SLOW_SPEED;
    }

    void setFastSpeed()
    {
        static constexpr float DEFAULT_FAST_SPEED = 0.4;
        speed = DEFAULT_FAST_SPEED;
    }

    MazeCell ghostJailTopCell = MazeCell(-1, -1);
    MazeCell ghostJailBottomCell = MazeCell(-1, -1);

protected:
    MazeCell* previousCell = nullptr;
    boost::optional<std::chrono::time_point<std::chrono::steady_clock>> ghostJailBounceStart = boost::none;

    static float getCellToCellDistance(const MazeCell* cell1, const MazeCell* cell2)
    {
        if (!cell1 || !cell2) return NAN;
        return sqrt(pow((cell2->gridX - cell1->gridX), 2)
            + pow((cell2->gridY - cell1->gridY), 2));
    }

    virtual MazeCell* getScatterCell() = 0;

    MazeCell* getClosestNeighborToTargetCell(const MazeCell* potentiallyUnwalkableTargetCell)
    {
        std::unordered_map<Direction, float> directionDistances;

        const auto* rightNeighbor = mazeFile->getCell(currentCell->gridX + 1, currentCell->gridY);
        const auto* leftNeighbor = mazeFile->getCell(currentCell->gridX - 1, currentCell->gridY);
        const auto* upNeighbor = mazeFile->getCell(currentCell->gridX, currentCell->gridY - 1);
        const auto* downNeighbor = mazeFile->getCell(currentCell->gridX, currentCell->gridY + 1);

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
        for (auto& [dir, dist] : directionDistances)
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


    MazeCell* scatterIfNecessary()
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
        static constexpr auto GHOST_JAIL_TIME = std::chrono::seconds(5);

        if (inSpawnJail)
        {
            if (!jailDotCount)
            {
                inSpawnJail = false;
                inJail = false;
                isDead = false;
                isScattering = false;
                setNormalSpeed();
                ghostJailBounceStart = boost::none;
                return mazeFile->getGhostJailEntryCell();
            }
        }

        if (!inSpawnJail && std::chrono::steady_clock::now() - *ghostJailBounceStart >= GHOST_JAIL_TIME)
        {
            inJail = false;
            isDead = false;
            isScattering = false;
            setNormalSpeed();
            ghostJailBounceStart = boost::none;
            return mazeFile->getGhostJailEntryCell();
        }

        if (*currentCell == *mazeFile->getGhostJailEntryCell())
        {
            setJailSpeed();
            currentCell = &ghostJailBottomCell;
            return &ghostJailTopCell;
        }

        if (*currentCell == ghostJailTopCell)
        {
            return &ghostJailBottomCell;
        }
        if (*currentCell == ghostJailBottomCell)
        {
            return &ghostJailTopCell;
        }
        return nullptr;
    }

    MazeCell* goToJail()
    {
        if (currentCell != mazeFile->getGhostJailEntryCell())
        {
            return getClosestNeighborToTargetCell(mazeFile->getGhostJailEntryCell());
        }

        // here we would enter the jail
        inJail = true;
        setSlowSpeed();
        ghostJailBounceStart = std::chrono::steady_clock::now();
        return bounceInJailUntilRespawn();
    }

private:
    void setJailSpeed()
    {
        speed = 0.05;
    }

    void init_ghost_jail_bounce_cells(const MazeCell *ghostJailBounceCell)
    {
        if (!mazeFile || !ghostJailBounceCell)
        {
            // TODO -> log properly lol
            return;
        }

        ghostJailTopCell = MazeCell(
            ghostJailBounceCell->gridX,
            ghostJailBounceCell->gridY,
            TileType::OPEN,
            ghostJailBounceCell->gridX * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS,
            ghostJailBounceCell->gridY * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS -
            NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS / 4
        );
        ghostJailBottomCell = MazeCell(
            ghostJailBounceCell->gridX,
            ghostJailBounceCell->gridY,
            TileType::OPEN,
            ghostJailBounceCell->gridX * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS,
            ghostJailBounceCell->gridY * NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS +
            NATIVE_RESOLUTION_TILE_GRID_SIZE_IN_PIXELS / 4
        );
    }
};
