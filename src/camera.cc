#include "camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : m_fov(glm::radians(45.0f))
    , m_aspectRatio(1.0f)
    , m_zNear(0.1f)
    , m_zFar(100.0f)
    , m_eye(glm::vec3(1, 0, 0))
    , m_center(glm::vec3(0, 0, 0))
    , m_up(glm::vec3(0, 1, 0))
{
    updateProjectionMatrix();
    updateViewMatrix();
}

void Camera::setFieldOfView(float fov)
{
    m_fov = glm::radians(fov);
    updateProjectionMatrix();
}

void Camera::setAspectRatio(float aspectRatio)
{
    m_aspectRatio = aspectRatio;
    updateProjectionMatrix();
}

void Camera::setZNear(float zNear)
{
    m_zNear = zNear;
    updateProjectionMatrix();
}

void Camera::setZFar(float zFar)
{
    m_zFar = zFar;
    updateProjectionMatrix();
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

void Camera::updateProjectionMatrix()
{
    m_projectionMatrix = glm::perspective(m_fov, m_aspectRatio, m_zNear, m_zFar);
}

void Camera::updateViewMatrix()
{
    m_viewMatrix = glm::lookAt(m_eye, m_center, m_up);
}
