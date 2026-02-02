#include "Material.h"
#include "Grid.h"

inline bool isDenser(const Pixel& p1, const Pixel& p2);

void updatePowder(Grid& g, int x, int y);
void updateLiquid(Grid& g, int x, int y);