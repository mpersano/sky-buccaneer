#include "geometryutils.h"

BoundingBox BoundingBox::operator|(const glm::vec3 &p) const
{
    BoundingBox b = *this;
    b |= p;
    return b;
}

BoundingBox &BoundingBox::operator|=(const glm::vec3 &p)
{
    min = glm::min(p, min);
    max = glm::max(p, max);
    return *this;
}
