#pragma once
#include "World.h"
#include <atomic>

class GameServer {
private:
    World world;
    std::atomic<bool> running;

public:
    GameServer();
    void start();
    void stop();
};