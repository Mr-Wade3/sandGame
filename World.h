#pragma once
#include <unordered_map>
#include <memory>
#include "Entity.h"

class World {
private:
    std::unordered_map<uint32_t, std::shared_ptr<Entity>> entities;

public:
    void addEntity(std::shared_ptr<Entity> entity) {
        entities[entity->getId()] = entity;
    }

    void removeEntity(uint32_t id) {
        entities.erase(id);
    }

    void update(float deltaTime) {
        for (auto& [id, entity] : entities) {
            entity->update(deltaTime);
        }
    }
};