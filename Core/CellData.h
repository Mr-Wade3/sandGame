#ifndef _CELLDATA_H_
#define _CELLDATA_H_

#pragma once

class CellData {
public:
    int x;
    int y;

    CellData()
    {
        x = 0;
        y = 0;
    }

    CellData(int p_x, int p_y)
    {
        x = p_x;
        y = p_y;
    }

    bool operator< (const CellData& other) const
    {
        if (y != other.y)
            return y < other.y;
        return x < other.x;
    }


    bool operator> (const CellData& other)
    {
        if (y != other.y)
            return y > other.y;
        return x > other.x;
    }
};
#endif