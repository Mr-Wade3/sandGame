#include "GameServer.h"
#include <chrono>
#include <thread>

GameServer::GameServer() : running(false) {}

void GameServer::start() {
    running = true;

    const float tickRate = 1.0f / 60.0f; // 60 TPS

    while (running) {
        auto start = std::chrono::high_resolution_clock::now();

        world.update(tickRate);

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = end - start;

        if (elapsed.count() < tickRate) {
            std::this_thread::sleep_for(
                std::chrono::duration<float>(tickRate - elapsed.count()));
        }
    }
}

void GameServer::stop() {
    running = false;
}