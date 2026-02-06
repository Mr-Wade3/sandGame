#include "Material.h"
#include <fstream>
#include <iostream>

#include "json.hpp" // nlohmann/json

#include "MaterialBehaviors.h"
#include "Colour.h"

using json = nlohmann::json;

// ------------------ Storage ------------------

static std::vector<MaterialDef> g_materials;
static std::unordered_map<std::string, MaterialID> g_nameToId;
static std::vector<MaterialPhaseChanges> phaseChanges;
MaterialID lastMaterial;

// ----------------- Pre-cache -----------------

MaterialID MAT_EMPTY;
MaterialID MAT_SAND;
MaterialID MAT_WATER;
MaterialID MAT_FIRE;
MaterialID MAT_STEAM;

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

static UpdateFn stateToUpdate(const MaterialState &state) {
    if (state == MaterialState::Solid)   return nullptr;
    if (state == MaterialState::Powder)  return updatePowder;
    if (state == MaterialState::Liquid)  return updateLiquid;
    if (state == MaterialState::Gas)     return updateGas;
    if (state == MaterialState::Plasma)  return updatePlasma;
}

// ------------------ Loader ------------------

void prepMaterials() {
    g_materials.clear();
    g_nameToId.clear();
    phaseChanges.clear();

    // Load "Empty" as it will always be needed
    MaterialDef Empty{};
    Empty.name = "Empty";
    Empty.id = (MaterialID)g_materials.size();
    

    Empty.state          = parseState("Gas");
    Empty.density        = 0;
    Empty.friction       = 0;
    Empty.dispersion     = 0;

    Empty.temperature    = roomTemp; // (273.1 + 20) * 10 
    Empty.ignitionTemp   = 0;
    Empty.meltTemp       = 0;
    Empty.boilTemp       = 0;

    Empty.burnsInto      = 0;
    Empty.boilsInto      = 0;
    Empty.meltsInto      = 0;
    Empty.solidifiesInto = 0;

    Empty.lifeTime       = -1;

    Empty.color          = parseColor("00000000");
    Empty.emissive       = false;

    Empty.flags |= (uint8_t)false << 0; // flammable
    Empty.flags |= (uint8_t)false << 1; // solid
    Empty.flags |= (uint8_t)false << 2; // destructible
    Empty.flags |= (uint8_t)false << 3; // consumes
    Empty.flags |= (uint8_t)false << 4; // canMelt
    Empty.flags |= (uint8_t)false << 5; // canBoil

    Empty.update = nullptr;

    g_nameToId[Empty.name] = Empty.id;
    g_materials.push_back(Empty);

    MaterialPhaseChanges MPC{};
    MPC.burnsInto      = "Empty";
    MPC.boilsInto      = "Empty";
    MPC.meltsInto      = "Empty";
    MPC.solidifiesInto = "Empty";

    phaseChanges.push_back(MPC);
}

bool loadMaterialsFromJSON(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "Failed to open material file: " << path << "\n";
        return false;
    }

    json j;
    f >> j;

    g_materials.reserve(j["materials"].size() + g_materials.size());
    g_nameToId.reserve(j["materials"].size() + g_nameToId.size());

    for (const auto& jm : j["materials"]) {
        MaterialDef m{};

        m.name = jm["name"];
        if (m.name == "Empty") continue;
        m.id = (MaterialID)g_materials.size();
        

        m.state          = parseState(jm["state"]);
        m.density        = jm.value("density", 0);
        m.friction       = jm.value("friction", 0);
        m.dispersion     = jm.value("dispersion", 0);

        m.temperature    = jm.value("temperature", roomTemp);
        m.ignitionTemp   = jm.value("ignitionTemp", 0);
        m.meltTemp       = jm.value("meltTemp", 0);
        m.boilTemp       = jm.value("boilTemp", 0);

        MaterialPhaseChanges MPC{};
        MPC.burnsInto      = jm.value("burnsInto", "Empty");
        MPC.boilsInto      = jm.value("boilsInto", "Empty");
        MPC.meltsInto      = jm.value("meltsInto", "Empty");
        MPC.solidifiesInto = jm.value("solidifiesInto", "Empty");

        m.lifeTime       = jm.value("lifeTime", -1);

        m.color          = parseColor(jm["color"]);
        m.emissive       = jm.value("emissive", false);

        m.flags         |= (uint8_t)jm.value("flammable", false)      << 0; // flammable
        m.flags         |= (uint8_t)(m.state == MaterialState::Solid) << 1; // solid
        m.flags         |= (uint8_t)jm.value("destructible", true)    << 2; // destructible
        m.flags         |= (uint8_t)jm.value("consumes", false)       << 3; // consumes
        m.flags         |= (uint8_t)jm.value("canMelt", false)        << 4; // canMelt
        m.flags         |= (uint8_t)jm.value("canBoil", false)        << 5; // canBoil
        // m.flammable    = jm.value("flammable", false);
        // m.solid        = jm.value("solid", false);
        // m.destructible = jm.value("destructible", false);

        m.update = stateToUpdate(m.state);

        g_nameToId[m.name] = m.id;
        g_materials.push_back(m);
        phaseChanges.push_back(MPC);
    }

    std::cout << "Loaded materials: " << g_nameToId.size() << "\n";

    
    lastMaterial = (MaterialID)(g_materials.size() - 1);

    return true;
}

void addPhaseChanges() {
    for (int id = 1; id < phaseChanges.size(); id++) {
        MaterialDef& material = g_materials[id];
        MaterialPhaseChanges PhaseChanges = phaseChanges[id];
        material.burnsInto      = materialIdByName(PhaseChanges.burnsInto);
        material.boilsInto      = materialIdByName(PhaseChanges.boilsInto);
        material.meltsInto      = materialIdByName(PhaseChanges.meltsInto);
        material.solidifiesInto = materialIdByName(PhaseChanges.solidifiesInto);
        if (material.state == MaterialState::Solid)   material.solidifiesInto = material.id;
        if (material.state == MaterialState::Powder)  material.solidifiesInto = material.id;
        if (material.state == MaterialState::Liquid)  material.meltsInto      = material.id;;
        if (material.state == MaterialState::Gas)     material.boilsInto      = material.id;
        if (material.state == MaterialState::Plasma)  material.burnsInto      = material.id;
        // std::cout << material.id << " " << material.name << "\n> " << material.burnsInto << "\n> " << material.boilsInto << "\n> " << material.meltsInto << "\n> " << material.solidifiesInto << "\n";
    }
}

void preCaching() {
    MAT_EMPTY = materialIdByName("Empty");
    MAT_SAND = materialIdByName("Sand");
    MAT_WATER = materialIdByName("Water");
    MAT_FIRE  = materialIdByName("Fire");
    MAT_STEAM = materialIdByName("Steam");
}

// ------------------ Access ------------------

const MaterialDef& mat(MaterialID id) {
    return g_materials[id];
}

MaterialID materialIdByName(const std::string& name) {
    return g_nameToId.at(name);
}


// ------------------ Material Behaviors ------------------

void tryIgnite(Pixel& p)
{
    const auto& m = mat(p.material);

    if (m.flags & CAN_IGNITE && p.temperature > m.ignitionTemp)
    {
        p.material = m.burnsInto;
        p.life = mat(p.material).lifeTime;
    }
}

bool exchangeHeat(Pixel& a, Pixel& b)
{
    Temperture avg = (a.temperature + b.temperature) >> 1; // bitshift left by one should be equent to devide by 2
    if (avg != a.temperature){
        a.temperature = avg;
        b.temperature = avg;
        return true;
    }
    return false;
}

void updatePhase(Pixel& p)
{
    const auto& m = mat(p.material);

    if (m.boilTemp > 0 && p.temperature > m.boilTemp) {
        p.material = m.boilsInto;
        return;
    }
    if (m.meltTemp > 0 && p.temperature > m.meltTemp) {
        p.material = m.meltsInto;
        return;
    }
    if (m.meltTemp > 0 && p.temperature < m.meltTemp) {
        p.material = m.solidifiesInto;
    }
}


// ------------------ Colour ------------------
Colour materialToColor(MaterialID id) {
    // aarrggbb
    uint32_t c = mat(id).color;
    return {
        (unsigned char)((c >> 16) & 0xFF),  // r
        (unsigned char)((c >> 8)  & 0xFF),  // g
        (unsigned char)(c & 0xFF),          // b
        (unsigned char)((c >> 24) & 0xFF)   // a
    };
}