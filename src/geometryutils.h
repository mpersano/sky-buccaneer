#pragma once

#include <glm/glm.hpp>

struct BoundingBox {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = glm::vec3(std::numeric_limits<float>::min());

    BoundingBox operator|(const glm::vec3 &p) const;
    BoundingBox &operator|=(const glm::vec3 &p);
};
