#pragma once
#include <cstdint>

enum class MaterialState {
    Solid,
    Powder,
    Liquid,
    Gas,
    Plasma
};

enum class MaterialID : uint16_t {
    Empty = 0,

    Sand,
    Stone,
    // Dirt,

    Water,
    // Oil,
    // Lava,

    // Smoke,
    // Steam,

    Fire,

    COUNT
};

struct MaterialDef {
    MaterialID id;
    const char* name;

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

extern MaterialDef Materials[(int)MaterialID::COUNT];

inline const MaterialDef& mat(MaterialID id);
bool isdenserThan(Pixel p1, Pixel p2);
bool isdenserThan(MaterialDef m1, MaterialDef m2);
void tryIgnite(Pixel& p);
void exchangeHeat(Pixel& a, Pixel& b);
void updatePhase(Pixel& p);