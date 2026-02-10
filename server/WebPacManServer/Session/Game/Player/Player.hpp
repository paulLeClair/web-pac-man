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

private:
    [[nodiscard]] MazeCell *getMazeCellIfWalkable(int x, int y) const;
    MazeCell* getNextWalkableCellByOrientation();
};

} // pacman
