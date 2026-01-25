#pragma once
#include <cstdint>

class Entity {
protected:
    uint32_t id;

public:
    Entity(uint32_t id) : id(id) {}
    virtual ~Entity() = default;

    uint32_t getId() const { return id; }

    virtual void update(float deltaTime) = 0;
};