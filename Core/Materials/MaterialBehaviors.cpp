#include <cstdlib>
#include "MaterialBehaviors.h"
#include "Rng.h"


inline bool isDenser(const Pixel& a, const Pixel& b) {
    const MaterialDef& m1 = mat(a.material);
    const MaterialDef& m2 = mat(b.material);
    return !(m1.flags & IS_SOLID || m2.flags & IS_SOLID) && m1.density > m2.density;
}

void updatePowder(Grid& g, int x, int y) {
    Pixel& p = g.at(x, y);

    if (g.inBounds(x, y + 1) && isDenser(p, g.at(x, y + 1))) {
        std::swap(p, g.at(x, y + 1));
        g.activeNext->addActive3x3(x, y);
        return;
    }

    int dir = randBit() & 1 ? -1 : 1;
    if (g.inBounds(x + dir, y + 1) &&
        isDenser(p, g.at(x + dir, y + 1))) {
        std::swap(p, g.at(x + dir, y + 1));
        g.activeNext->addActive3x3(x, y);
        return;
    }

    if (g.inBounds(x - dir, y + 1) &&
        isDenser(p, g.at(x - dir, y + 1))) {
        std::swap(p, g.at(x - dir, y + 1));
        g.activeNext->addActive3x3(x, y);
    }
}

void updateLiquid(Grid& g, int x, int y) {
    Pixel& p = g.at(x, y);

    if (g.inBounds(x, y + 1) && isDenser(p, g.at(x, y + 1))) {
        std::swap(p, g.at(x, y + 1));
        g.activeNext->addActive3x3(x, y);
        return;
    }

    int dir = randBit() & 1 ? -1 : 1;
    if (g.inBounds(x + dir, y) &&
        g.at(x + dir, y).material == MAT_EMPTY) {
        std::swap(p, g.at(x + dir, y));
        g.activeNext->addActive3x3(x, y);
        return;
    }

    if (g.inBounds(x - dir, y) &&
        g.at(x - dir, y).material == MAT_EMPTY) {
        std::swap(p, g.at(x - dir, y));
        g.activeNext->addActive3x3(x, y);
    }
}

void updateGas(Grid& g, int x, int y) {
    Pixel& p = g.at(x, y);

    // Rise
    if (g.inBounds(x, y - 1) && isDenser(g.at(x, y - 1), p)) {
        std::swap(p, g.at(x, y - 1));
        g.activeNext->addActive3x3(x, y);
        return;
    }

    // Diagonal rise
    int dir = randBit() ? -1 : 1;
    if (g.inBounds(x + dir, y - 1) &&
        g.at(x + dir, y - 1).material == MAT_EMPTY) {
        std::swap(p, g.at(x + dir, y - 1));
        g.activeNext->addActive3x3(x, y);
        return;
    }

    if (g.inBounds(x - dir, y - 1) &&
        g.at(x - dir, y - 1).material == MAT_EMPTY) {
        std::swap(p, g.at(x - dir, y - 1));
        g.activeNext->addActive3x3(x, y);
        return;
    }

    // Horizontal diffusion
    if (g.inBounds(x + dir, y) &&
        g.at(x + dir, y).material == MAT_EMPTY) {
        std::swap(p, g.at(x + dir, y));
        g.activeNext->addActive3x3(x, y);
    }

    if (g.inBounds(x - dir, y) &&
        g.at(x - dir, y).material == MAT_EMPTY) {
        std::swap(p, g.at(x - dir, y));
        g.activeNext->addActive3x3(x, y);
    }
}

void updatePlasma(Grid& g, int x, int y) {
    Pixel& p = g.at(x, y);

    // Random movement direction
    int dx = (randBit() ? -1 : 1);
    int dy = (randBit() ? -1 : 1);

    // Prefer upward chaos
    if (randBit()) dy = -1;

    int nx = x + dx;
    int ny = y + dy;

    if (g.inBounds(nx, ny)) {
        Pixel& t = g.at(nx, ny);

        // Burn or displace almost anything
        if (mat(t.material).state != MaterialState::Plasma) {
            // Example interaction
            // if (t.material == MAT_WATER) {
            //     t.material = MAT_STEAM;
            // } else 
            if (t.material == MAT_EMPTY) {
                std::swap(p, t);
            } 
            // else {
            //     // Convert solids/liquids to fire/ash/etc
            //     t.material = MAT_FIRE;
            // }

            g.activeNext->addActive3x3(x, y);
        }
    }

    // Plasma decays
    if ((rand() & 15) == 0) {
        p.material = MAT_EMPTY;
        g.activeNext->addActive3x3(x, y);
    }
}
