#include "collisionmesh.h"

CollisionMesh::CollisionMesh() = default;

CollisionMesh::CollisionMesh(const std::vector<Triangle> &triangles)
{
    addTriangles(triangles);
}

void CollisionMesh::addTriangles(const std::vector<Triangle> &triangles)
{
    m_triangles.insert(m_triangles.end(), triangles.begin(), triangles.end());
    for (const auto &triangle : triangles) {
        m_boundingBox |= triangle.v0;
        m_boundingBox |= triangle.v1;
        m_boundingBox |= triangle.v2;
    }
}

std::optional<float> CollisionMesh::intersection(const LineSegment &segment) const
{
    if (!segment.intersects(m_boundingBox))
        return {};
    std::optional<float> collisionT;
    for (const auto &triangle : m_triangles) {
        if (const auto ot = segment.intersection(triangle)) {
            if (const auto t = *ot; !collisionT || t < collisionT) {
                collisionT = t;
            }
        }
    }
    return collisionT;
}
