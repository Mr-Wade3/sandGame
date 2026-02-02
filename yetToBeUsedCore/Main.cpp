#include "GameServer.h"
#include "Player.h"
#include <memory>

int main() {
    GameServer server;

    // Example player creation
    auto player = std::make_shared<Player>(1, "PlayerOne");

    server.start();

    return 0;
}