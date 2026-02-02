#pragma once
#include <vector>
#include <cstdint>
#include "Pixel.h"
#include "Active.h"

class Grid {
public:
    Grid(int w, int h);

    Pixel& at(int x, int y);
    inline int ix(int i) { return i % width; }
    inline int iy(int i) { return i / width; }
    bool inBounds(int x, int y) const;

    void swapActive();

    int width, height;
    std::vector<Pixel> cells;
    Active* activeNow;
    Active* activeNext;
};