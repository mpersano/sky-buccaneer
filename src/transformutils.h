#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

glm::mat4 composeTransformMatrix(const glm::vec3 &translation, const glm::quat &rotation, const glm::vec3 &scale);
