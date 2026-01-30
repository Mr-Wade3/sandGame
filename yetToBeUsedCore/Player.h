#pragma once
#include "Entity.h"
#include <string>

struct Vec2 {
    float x, y;
};

class Player : public Entity {
private:
    std::string name;
    Vec2 position;
    int health;

public:
    Player(uint32_t id, const std::string& name)
        : Entity(id), name(name), position{0, 0}, health(100) {}

    void move(float dx, float dy) {
        position.x += dx;
        position.y += dy;
    }

    void update(float deltaTime) override {
        // Apply server-side logic (regen, cooldowns, etc.)
    }

    const Vec2& getPosition() const { return position; }
    int getHealth() const { return health; }
};