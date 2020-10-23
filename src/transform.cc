#include "transform.h"

#include "datastream.h"
#include "transformutils.h"

DataStream &operator>>(DataStream &ds, Transform &t)
{
    ds >> t.translation;
    ds >> t.rotation;
    ds >> t.scale;
    return ds;
}

glm::mat4 Transform::matrix() const
{
    return composeTransformMatrix(translation, rotation, scale);
}
