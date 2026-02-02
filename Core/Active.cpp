#include "Active.h"

Active::Active(int w, int h) : width(w), height(h), activeMask(w * h, 0) {
    active.reserve((w * h) >> 2);
}

void Active::addActive(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return;

    int i = y * width + x;
    if (!activeMask[i]) {
        activeMask[i] = 1;
        active.push_back(i);
    }
}

void Active::clearActive() {
    for (int i : active)
        activeMask[i] = 0;
    active.clear();
}

void Active::addActive3x3(int x, int y) {
    int xn = x - 1;
    int xp = x + 1;
    int yn = y - 1;
    int yp = y + 1;
    
    addActive(xp, yp); //se
    addActive(x , yp); //s
    addActive(xn, yp); //sw
    addActive(xp, y ); // e
    addActive(x , y ); // c
    addActive(xn, y ); // w
    addActive(xp, yn); //ne
    addActive(x , yn); //n
    addActive(xn, yn); //nw
}