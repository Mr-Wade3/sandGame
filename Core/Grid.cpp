#include "Grid.h"

Grid::Grid(int w, int h) : width(w), height(h), cells(w * h, {(MaterialID)0, roomTemp, -1})
{
    activeNow  = new Active(w, h);
    activeNext = new Active(w, h);
}


Pixel& Grid::at(int x, int y) {
    return cells[y * width + x];
}

bool Grid::inBounds(int x, int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

void Grid::swapActive() {
    std::swap(activeNow, activeNext);
}