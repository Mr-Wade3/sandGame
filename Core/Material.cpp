#include "Material.h"
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "json.hpp" // nlohmann/json

using json = nlohmann::json;

// ------------------ Storage ------------------

static std::vector<MaterialDef> g_materials;
static std::unordered_map<std::string, MaterialID> g_nameToId;
uint16_t numOfMaterials;

// ----------------- Pre-cache -----------------

MaterialID MAT_EMPTY;
MaterialID MAT_SAND;
MaterialID MAT_WATER;
MaterialID MAT_FIRE;

// ------------------ Helpers ------------------
   
static MaterialState parseState(const std::string& s) {
    if (s == "Solid")   return MaterialState::Solid;
    if (s == "Powder")  return MaterialState::Powder;
    if (s == "Liquid")  return MaterialState::Liquid;
    if (s == "Gas")     return MaterialState::Gas;
    if (s == "Plasma")  return MaterialState::Plasma;

    throw std::runtime_error("Unknown MaterialState: " + s);
}

static uint32_t parseColor(const std::string& hex) {
    // expects "FFAABBCC" or "AARRGGBB"
    return std::stoul(hex, nullptr, 16);
}

// ------------------ Loader ------------------

bool loadMaterialsFromJSON(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "Failed to open material file: " << path << "\n";
        return false;
    }

    json j;
    f >> j;

    g_materials.clear();
    g_nameToId.clear();

    g_materials.reserve(j["materials"].size() + 1);
    g_nameToId.reserve(j["materials"].size() + 1);

    // Load "Empty" as it will always be needed
    MaterialDef Empty{};
    Empty.name = "Empty";
    Empty.id = (MaterialID)g_materials.size();
    

    Empty.state        = parseState("Gas");
    Empty.density      = 0;
    Empty.friction     = 0.0f;
    Empty.dispersion   = 0.0f;

    Empty.temperature  = roomTemp; // (273.1 + 20) * 10 
    Empty.ignitionTemp = 0;
    Empty.meltTemp     = 0;
    Empty.boilTemp     = 0;

    Empty.lifeTime     = -1;

    Empty.color        = parseColor("00000000");
    Empty.emissive     = false;

    Empty.flags |= (uint8_t)false << 0; // flammable
    Empty.flags |= (uint8_t)false << 1; // solid
    Empty.flags |= (uint8_t)false << 2; // destructible
    Empty.flags |= (uint8_t)false << 3; // consumes

    g_nameToId[Empty.name] = Empty.id;
    g_materials.push_back(Empty);

    for (const auto& jm : j["materials"]) {
        MaterialDef m{};

        m.name = jm["name"];
        if (m.name == "Empty") continue;
        m.id = (MaterialID)g_materials.size();
        

        m.state        = parseState(jm["state"]);
        m.density      = jm.value("density", 0);
        m.friction     = jm.value("friction", 0.0f);
        m.dispersion   = jm.value("dispersion", 0.0f);

        m.temperature  = jm.value("temperature", roomTemp);
        m.ignitionTemp = jm.value("ignitionTemp", 0.0f);
        m.meltTemp     = jm.value("meltTemp", 0.0f);
        m.boilTemp     = jm.value("boilTemp", 0.0f);

        m.lifeTime     = jm.value("lifeTime", -1);

        m.color        = parseColor(jm["color"]);
        m.emissive     = jm.value("emissive", false);

        m.flags        |= (uint8_t)jm.value("flammable", false)    << 0; // flammable
        m.flags        |= (uint8_t)jm.value("solid", false)        << 1; // solid
        m.flags        |= (uint8_t)jm.value("destructible", false) << 2; // destructible
        m.flags        |= (uint8_t)jm.value("consumes", false)     << 3; // consumes
        // m.flammable    = jm.value("flammable", false);
        // m.solid        = jm.value("solid", false);
        // m.destructible = jm.value("destructible", false);

        g_nameToId[m.name] = m.id;
        g_materials.push_back(m);
    }

    std::cout << "Loaded materials: " << g_nameToId.size() << "\n";

    MAT_EMPTY = materialIdByName("Empty");
    MAT_SAND = materialIdByName("Sand");
    MAT_WATER = materialIdByName("Water");
    MAT_FIRE  = materialIdByName("Fire");
    
    numOfMaterials = (uint16_t)g_materials.size();

    return true;
}

// ------------------ Access ------------------

const MaterialDef& mat(MaterialID id) {
    return g_materials[id];
}

MaterialID materialIdByName(const std::string& name) {
    return g_nameToId.at(name);
}

bool isdenserThan(Pixel& p1, Pixel& p2)
{
    const MaterialDef& m1 = mat(p1.material);
    const MaterialDef& m2 = mat(p2.material);
    return m1.density > m2.density;
}

bool isdenserThan(MaterialDef m1, MaterialDef m2)
{
    return m1.density > m2.density;
}

// ------------------ Simulation ------------------

void tryIgnite(Pixel& p)
{
    const auto& m = mat(p.material);

    if (m.flags & CAN_IGNITE && p.temperature > m.ignitionTemp)
    {
        p.material = MAT_FIRE;
        p.life = mat(p.material).lifeTime;
    }
}

void exchangeHeat(Pixel& a, Pixel& b)
{
    float avg = (a.temperature + b.temperature) * 0.5f;
    a.temperature = avg;
    b.temperature = avg;
}

// void updatePhase(Pixel& p)
// {
//     const auto& m = mat(p.material);

//     if (m.boilTemp > 0 && p.temperature > m.boilTemp)
//         p.material = materialIdByName("Steam");

//     if (m.meltTemp > 0 && p.temperature > m.meltTemp)
//         p.material = materialIdByName("Lava");
// }