#pragma once
#include "Materials\MaterialID.h"
#include "Temperture.h"

struct Pixel {
    MaterialID material;
    Temperture  temperature; // temperature in tenths Kelvin
    int life;
};