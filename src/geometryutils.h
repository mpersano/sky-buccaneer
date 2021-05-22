#pragma once

#include <glm/glm.hpp>

#include <optional>

struct BoundingBox {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());

    bool contains(const glm::vec3 &p) const;
    BoundingBox operator|(const glm::vec3 &p) const;
    BoundingBox &operator|=(const glm::vec3 &p);
};

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;
};

struct Triangle {
    glm::vec3 v0, v1, v2;
};

std::optional<float> rayTriangleIntersection(const Ray &ray, const Triangle &triangle);
