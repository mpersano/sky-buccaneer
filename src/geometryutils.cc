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

glm::vec3 LineSegment::pointAt(float t) const
{
    return (1.0f - t) * from + t * to;
}

std::optional<float> LineSegment::intersection(const Triangle &triangle) const
{
    return triangle.intersection(*this);
}

Ray LineSegment::ray() const
{
    return { from, to - from };
}

glm::vec3 Ray::pointAt(float t) const
{
    return origin + t * direction;
}

std::optional<float> Ray::intersection(const Triangle &triangle) const
{
    return triangle.intersection(*this);
}

std::optional<float> Triangle::intersection(const LineSegment &segment) const
{
    auto ot = intersection(segment.ray());
    if (!ot)
        return {};
    const auto t = *ot;
    if (t > 1.0f)
        return {};
    return t;
}

// Moller-Trumbore
std::optional<float> Triangle::intersection(const Ray &ray) const
{
    constexpr const auto Epsilon = 1e-6;

    const auto e1 = v1 - v0;
    const auto e2 = v2 - v0;

    const auto h = glm::cross(ray.direction, e2);
    const auto a = glm::dot(e1, h);
    if (std::fabs(a) < Epsilon)
        return {}; // parallel to triangle

    const auto f = 1.0f / a;
    const auto s = ray.origin - v0;
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
