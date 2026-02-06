#include <stdlib.h>
#include <iostream>

#include <Simulation.h>
#include <Materials\Material.h>
#include <Grid.h>
#include <Renderer.h>
#include <Player.h>


// const int WIDTH = 1920;
// const int HEIGHT = 1080;
const int WIDTH = 1600;
const int HEIGHT = 800;
const int SCALE = 4;
const int GRID_WIDTH = WIDTH / SCALE;
const int GRID_HEIGHT = HEIGHT / SCALE;




Grid grid(GRID_WIDTH, GRID_HEIGHT);
Simulation sim(grid);
Renderer renderer(WIDTH, HEIGHT);
Player mainPlayer(0,"player");

int main() {

    prepMaterials();
    // if (!loadMaterialsFromJSON("materials\\materials.json")) return -1;
    if (!loadMaterialsFromJSON("materials\\gases.json")) return -1;
    if (!loadMaterialsFromJSON("materials\\liquids.json")) return -1;
    if (!loadMaterialsFromJSON("materials\\powders.json")) return -1;
    if (!loadMaterialsFromJSON("materials\\solids.json")) return -1;
    if (!loadMaterialsFromJSON("materials\\plasmas.json")) return -1;
    addPhaseChanges();

    preCaching();

    renderer.prepWindow(grid);

    while (!renderer.shouldWindowClose()) {
        
        if (!renderer.Pawsed()) {
            sim.step();
        }

        renderer.Render(grid);
        // std::shuffle(active.begin(), active.end(), rng);
    }

    renderer.Terminate();
    return 0;
}
