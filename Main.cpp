#include <stdlib.h>
#include <iostream>

#include <Simulation.h>
#include <Material.h>
#include <Grid.h>
#include <Renderer.h>


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


int main() {

    prepMaterials();
    if (!loadMaterialsFromJSON("materials.json")) return -1;

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
