#include "BoundariesExtractor.h"

bool BoundariesExtractor::is_contour(int i) {
    return (mesh.EF()(i, 0) == -1 or mesh.EF()(i, 1) == -1);
}


