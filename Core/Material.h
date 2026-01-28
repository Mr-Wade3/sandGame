#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

enum class MaterialState {
    Solid,
    Powder,
    Liquid,
    Gas,
    Plasma
};

using MaterialID = uint16_t;
extern MaterialID MAT_EMPTY;
extern MaterialID MAT_SAND;
extern MaterialID MAT_WATER;
extern MaterialID MAT_FIRE;
extern uint16_t numOfMaterials;

struct MaterialDef {
    MaterialID id;
    std::string name;

    // Physics
    MaterialState state;
    int density;          // higher sinks
    float friction;       // movement resistance
    float dispersion;     // liquid spread

    // Thermal
    float temperature;
    float ignitionTemp;
    float meltTemp;
    float boilTemp;

    // Lifespan
    bool consumes;        // burns away?
    int lifeTime;         // frames (-1 = infinite)

    // Rendering
    uint32_t color;
    bool emissive;

    // Interaction flags
    bool flammable;
    bool solid;
    bool destructible;
};

struct Pixel {
    MaterialID material;
    float temperature;
    int life;
    bool updated;
};

// ------------------ API ------------------

bool loadMaterialsFromJSON(const std::string& path);


inline const MaterialDef& mat(MaterialID id);
MaterialID materialIdByName(const std::string& name);

bool isdenserThan(Pixel p1, Pixel p2);
bool isdenserThan(MaterialDef m1, MaterialDef m2);
void tryIgnite(Pixel& p);
void exchangeHeat(Pixel& a, Pixel& b);
void updatePhase(Pixel& p);