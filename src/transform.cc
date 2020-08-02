#include "transform.h"

#include "transformutils.h"

glm::mat4 Transform::matrix() const
{
    return composeTransformMatrix(translation, rotation, scale);
}
