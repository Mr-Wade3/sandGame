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