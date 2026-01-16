//
// Created by paull on 2026-01-15.
//

#include "Session.hpp"

namespace pacman {


    Session::Session(tcp::socket socket) : sessionId(boost::uuids::uuid()), socket(std::move(socket))
    {

    }
} // pacman