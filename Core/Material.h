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

const uint16_t roomTemp = 2931; // (273.1 + 20) * 10 

extern uint16_t numOfMaterials;

struct MaterialDef {
    MaterialID id;
    std::string name;
    MaterialState state;

    // Physics
    int density;          // higher sinks
    float friction;       // movement resistance
    float dispersion;     // liquid spread

    // Thermal
    uint16_t temperature;
    uint16_t ignitionTemp;
    uint16_t meltTemp;
    uint16_t boilTemp;

    MaterialID burnsInto;
    MaterialID boilsInto;
    MaterialID meltsInto;
    MaterialID solidifiesInto;


    // Lifespan
    int lifeTime;         // frames (-1 = infinite)

    // Rendering
    uint32_t color;
    bool emissive;

    // Interaction flags
    uint8_t flags = 0x0;
    // bool consumes;        // burns away?
    // bool flammable;
    // bool solid;
    // bool destructible;
};

enum MaterialBehaviorFlags : uint8_t {
    CAN_IGNITE  = 1 << 0,
    CAN_MELT    = 1 << 1,
    CAN_BOIL    = 1 << 2,
    IS_CONSUMED = 1 << 3
};

struct Pixel {
    MaterialID material;
    uint16_t  temperature; // temperature in tenths Kelvin
    int life;
};

// ------------------ API ------------------

bool loadMaterialsFromJSON(const std::string& path);


inline const MaterialDef& mat(MaterialID id);
MaterialID materialIdByName(const std::string& name);

bool isdenserThan(Pixel& p1, Pixel& p2);
bool isdenserThan(MaterialDef m1, MaterialDef m2);
void tryIgnite(Pixel& p);
void exchangeHeat(Pixel& a, Pixel& b);
void updatePhase(Pixel& p);