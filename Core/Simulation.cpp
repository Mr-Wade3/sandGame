#include "Simulation.h"
#include "Material.h"


Simulation::Simulation(Grid& grid) : g(grid) {}

void Simulation::step() {

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

        if (p.life > 0 && --p.life == 0) {
            p.material = MAT_EMPTY;
        }
        // if (updated) g.activeNext->addActive3x3(x, y);
    }
    g.activeNow->active.clear();
    g.swapActive();
}