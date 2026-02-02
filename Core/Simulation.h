#pragma once
#include "Grid.h"


class Simulation {
public:
    Simulation(Grid& grid);
    void step();

private:
    Grid& g;
};