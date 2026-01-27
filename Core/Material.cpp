#include "Material.h"

MaterialDef Materials[(int)MaterialID::COUNT] = {
    // Empty
    {
        MaterialID::Empty,
        "Empty",
        MaterialState::Gas,
        0, 0.0f, 0.0f,
        20.0f, 0.0f, 0.0f, 0.0f,
        false, -1,
        0x00000000,
        false,
        false, false, false
    },

    // Sand
    {
        MaterialID::Sand,
        "Sand",
        MaterialState::Powder,
        5, 0.9f, 0.1f,
        20.0f, 0.0f, 1700.0f, 0.0f,
        false, -1,
        0xFFCCB233,
        false,
        false, true, true
    },

    // Stone
    {
        MaterialID::Stone,
        "Stone",
        MaterialState::Solid,
        10, 0.9f, 0.0f,
        20.0f, 0.0f, 1700.0f, 0.0f,
        false, -1,
        0xFF808080,
        false,
        false, true, true
    },

    // Water
    {
        MaterialID::Water,
        "Water",
        MaterialState::Liquid,
        3, 0.02f, 1.0f,
        20.0f, 0.0f, 0.0f, 100.0f,
        false, -1,
        0xFF33B2CC,
        false,
        false, false, true
    },

    // Fire
    {
        MaterialID::Fire,
        "Fire",
        MaterialState::Plasma,
        0, 0.0f, 0.0f,
        800.0f, 0.0f, 0.0f, 0.0f,
        true, 60,
        0xFFFF5500,
        true,
        false, false, false
    }
};

inline const MaterialDef& mat(MaterialID id) {
    return Materials[(int)id];
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



void tryIgnite(Pixel& p)
{
    const auto& m = mat(p.material);

    if (m.flammable && p.temperature > m.ignitionTemp)
    {
        p.material = MaterialID::Fire;
        p.life = mat(MaterialID::Fire).lifeTime;
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