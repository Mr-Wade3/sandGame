#pragma once

class Colour {
public:
    inline Colour(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha) : r(red), g(green), b(blue), a(alpha) {};
    unsigned char r, g, b, a;
};