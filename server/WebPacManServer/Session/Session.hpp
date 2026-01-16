//
// Created by paull on 2026-01-15.
//

#pragma once

#include "../WebPacManServer.hpp"

#include "boost/uuid/uuid.hpp"

namespace pacman {

class Session {
public:
    explicit Session(tcp::socket socket);

    ~Session() = default;

private:
    boost::uuids::uuid sessionId;

    tcp::socket socket;

};

} // pacman
