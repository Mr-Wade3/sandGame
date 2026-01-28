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

    // note to self try hardcoding empty as you'll probably always need it
    // like air in minecraft

    g_materials.clear();
    g_nameToId.clear();

    // Load "Empty" as it will always be needed
    MaterialDef Empty{};
    Empty.name = "Empty";
    Empty.id = (MaterialID)g_materials.size();
    

    Empty.state        = parseState("Gas");
    Empty.density      = 0;
    Empty.friction     = 0.0f;
    Empty.dispersion   = 0.0f;

    Empty.temperature  = 20.0f;
    Empty.ignitionTemp = 0.0f;
    Empty.meltTemp     = 0.0f;
    Empty.boilTemp     = 0.0f;

    Empty.consumes     = false;
    Empty.lifeTime     = -1;

    Empty.color        = parseColor("00000000");
    Empty.emissive     = false;

    Empty.flammable    = false;
    Empty.solid        = false;
    Empty.destructible = false;

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

        m.temperature  = jm.value("temperature", 20.0f);
        m.ignitionTemp = jm.value("ignitionTemp", 0.0f);
        m.meltTemp     = jm.value("meltTemp", 0.0f);
        m.boilTemp     = jm.value("boilTemp", 0.0f);

        m.consumes     = jm.value("consumes", false);
        m.lifeTime     = jm.value("lifeTime", -1);

        m.color        = parseColor(jm["color"]);
        m.emissive     = jm.value("emissive", false);

        m.flammable    = jm.value("flammable", false);
        m.solid        = jm.value("solid", false);
        m.destructible = jm.value("destructible", false);

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
    std::cout << "Looking up material: " << name << " ,ID = " << g_nameToId.at(name)
              << " (map size = " << g_nameToId.size() << ")\n";
    return g_nameToId.at(name);
}

bool isdenserThan(Pixel p1, Pixel p2)
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

    if (m.flammable && p.temperature > m.ignitionTemp)
    {
        p.material = materialIdByName("Fire");
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
//         p.material = MaterialID::Steam;

//     if (m.meltTemp > 0 && p.temperature > m.meltTemp)
//         p.material = MaterialID::Lava;
// }