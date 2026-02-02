#pragma once
#include <vector>
#include <cstdint>
class Active {
public:
    Active(int w, int h);

    void addActive(int x, int y);
    void addActive3x3(int x, int y);
    void clearActive();
    

    int width, height;
    std::vector<int> active;
    std::vector<uint8_t> activeMask;
};