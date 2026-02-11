//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../../Defines.hpp"

namespace pacman {

struct Pacman final : Entity
{
    MazeFile *mazeFile;

    bool isChomping = false;
    Direction bufferedInput = Direction::DOWN;

    ~Pacman() override = default;
    MazeCell* obtainNextTarget() override;
    void update() override;

    void reverseDirection() {
        switch (orientation)
        {
        case Direction::UP: orientation = Direction::DOWN; break;
        case Direction::DOWN: orientation = Direction::UP; break;
        case Direction::LEFT: orientation = Direction::RIGHT; break;
        case Direction::RIGHT: orientation = Direction::LEFT; break;
        default: return;
        }

        param = 1.0f - param;
        const auto tmp = targetCell;
        targetCell = currentCell;
        currentCell = tmp;
    }

private:
    [[nodiscard]] MazeCell *getMazeCellIfWalkable(int x, int y) const;
    MazeCell* getNextWalkableCellByOrientation();


};

} // pacman
