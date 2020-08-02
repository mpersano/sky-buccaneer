#include "transform.h"

glm::mat4 Transform::matrix() const
{
    const auto r = glm::mat3_cast(rotation);
    glm::mat4 m;
    m[0][0] = scale.x * r[0][0]; m[1][0] = scale.y * r[1][0]; m[2][0] = scale.z * r[2][0]; m[3][0] = translation.x;
    m[0][1] = scale.x * r[0][1]; m[1][1] = scale.y * r[1][1]; m[2][1] = scale.z * r[2][1]; m[3][1] = translation.y;
    m[0][2] = scale.x * r[0][2]; m[1][2] = scale.y * r[1][2]; m[2][2] = scale.z * r[2][2]; m[3][2] = translation.z;
    m[0][3] = 0; m[1][3] = 0; m[2][3] = 0; m[3][3] = 1;
    return m;
}
