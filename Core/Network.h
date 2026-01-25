#pragma once
#include <string>

struct ClientMessage {
    uint32_t playerId;
    float moveX;
    float moveY;
};

class Network {
public:
    void poll() {
        // Receive client packets
    }

    void sendWorldState() {
        // Broadcast state to clients
    }
};