#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class DataStream;

struct Transform {
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;

    glm::mat4 matrix() const;
};

DataStream &operator>>(DataStream &ds, Transform &t);
