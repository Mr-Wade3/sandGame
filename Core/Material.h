#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>


#include "MaterialID.h"
#include "Temperture.h"
#include "Pixel.h"

enum class MaterialState {
    Solid,
    Powder,
    Liquid,
    Gas,
    Plasma
};


extern MaterialID MAT_EMPTY;
extern MaterialID MAT_SAND;
extern MaterialID MAT_WATER;
extern MaterialID MAT_FIRE;

extern MaterialID lastMaterial;

class Grid;
using UpdateFn = void (*)(Grid &, int, int);

struct MaterialDef {
    MaterialID id;
    std::string name;
    MaterialState state;

    // Physics
    int density;          // higher sinks
    uint8_t friction;       // movement resistance
    uint8_t dispersion;     // liquid spread

    // Thermal
    Temperture temperature;
    Temperture ignitionTemp;
    Temperture meltTemp;
    Temperture boilTemp;

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
    // bool flammable;
    // bool solid;
    // bool destructible;
    // bool consumes;        // burns away?
    // bool canMelt;
    // bool canBoil;
    
    UpdateFn update;
};

enum MaterialBehaviorFlags : uint8_t {
    CAN_IGNITE      = 1 << 0,
    IS_SOLID        = 1 << 1,
    IS_DESTRUCTIBLE = 1 << 2,
    IS_CONSUMED     = 1 << 3,
    CAN_MELT        = 1 << 4,
    CAN_BOIL        = 1 << 5
};



// ------------------ API ------------------

void prepMaterials();

bool loadMaterialsFromJSON(const std::string& path);


const MaterialDef& mat(MaterialID id);
MaterialID materialIdByName(const std::string& name);

void tryIgnite(Pixel& p);
void exchangeHeat(Pixel& a, Pixel& b);
void updatePhase(Pixel& p);

class Colour;
Colour materialToColor(MaterialID id);