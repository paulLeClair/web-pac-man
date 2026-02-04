//
// Created by paull on 2026-01-20.
//

#pragma once

#include "../../Defines.hpp"

namespace pacman {

struct Pacman : Entity
{
    // these match up with the client-side stuff
    enum class PacmanOrientation : int32_t
    {
        UP,
         DOWN,
         LEFT,
         RIGHT
    };

    bool isChomping;
    PacmanOrientation orientation;


};

} // pacman
