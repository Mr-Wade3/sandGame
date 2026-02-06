#include "Simulation.h"
#include "Materials\Material.h"
#include "XorShift32.h"

#include <algorithm>

Simulation::Simulation(Grid& grid) : g(grid) {}

static XorShift32 rng;

void Simulation::step() {

    shuffle(std::begin(g.activeNow->active), std::end(g.activeNow->active), rng);

    for (int i : g.activeNow->active) {
        g.activeNow->activeMask[i] = 0;
        int x = i % g.width;
        int y = i / g.width;

        if (!g.inBounds(x, y)) continue;

        Pixel& p = g.at(x, y);
        const MaterialDef& m = mat(p.material);

        if (m.update) {
            m.update(g, x, y);
        }

        if (g.inBounds(x, y + 1)) {
            if (exchangeHeat(p, g.at(x, y + 1))) {
                g.activeNext->addActive3x3(x, y);
            }
        }
        if (g.inBounds(x + 1, y)) {
            if (exchangeHeat(p, g.at(x + 1, y))) {
                g.activeNext->addActive3x3(x, y);
            }
        }

        if (p.life > 0) {
            g.activeNext->addActive3x3(x, y);
            if (--p.life == 0) {
                p.material = MAT_EMPTY;
            }
        }

        updatePhase(p);

        // if (updated) g.activeNext->addActive3x3(x, y);
    }
    g.activeNow->active.clear();
    g.swapActive();
}