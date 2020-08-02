#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : m_eye(glm::vec3(1, 0, 0))
    , m_center(glm::vec3(0, 0, 0))
    , m_up(glm::vec3(0, 1, 0))
{
    updateViewMatrix();
}

void Camera::setEye(const glm::vec3 &eye)
{
    m_eye = eye;
    updateViewMatrix();
}

void Camera::setCenter(const glm::vec3 &center)
{
    m_center = center;
    updateViewMatrix();
}

void Camera::setUp(const glm::vec3 &up)
{
    m_up = up;
    updateViewMatrix();
}

void Camera::updateViewMatrix()
{
    m_viewMatrix = glm::lookAt(m_eye, m_center, m_up);
}
