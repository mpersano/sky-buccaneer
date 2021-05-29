#pragma once

#include "geometryutils.h"

#include <vector>

class CollisionMesh
{
public:
    CollisionMesh();
    CollisionMesh(const std::vector<Triangle> &triangles);

    void addTriangles(const std::vector<Triangle> &triangles);
    std::optional<float> intersection(const LineSegment &segment) const;

private:
    BoundingBox m_boundingBox;
    std::vector<Triangle> m_triangles;
};
