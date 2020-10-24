#include "geometryutils.h"

bool BoundingBox::contains(const glm::vec3 &p) const
{
    constexpr auto Epsilon = 1e-6;
    return p.x > min.x - Epsilon && p.x < max.x + Epsilon &&
            p.y > min.y - Epsilon && p.y < max.y + Epsilon &&
            p.z > min.z - Epsilon && p.z < max.z + Epsilon;
}

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
