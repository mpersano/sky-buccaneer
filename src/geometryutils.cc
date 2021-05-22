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

// Moller-Trumbore
std::optional<float> rayTriangleIntersection(const Ray &ray, const Triangle &triangle)
{
    constexpr const auto Epsilon = 1e-6;

    const auto e1 = triangle.v1 - triangle.v0;
    const auto e2 = triangle.v2 - triangle.v0;

    const auto h = glm::cross(ray.direction, e2);
    const auto a = glm::dot(e1, h);
    if (std::fabs(a) < Epsilon)
        return {}; // parallel to triangle

    const auto f = 1.0f / a;
    const auto s = ray.origin - triangle.v0;
    const auto u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return {};

    const auto q = glm::cross(s, e1);
    const auto v = f * glm::dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f)
        return {};

    const auto t = f * glm::dot(e2, q);
    if (t < 0.0f)
        return {};

    return t;
}
