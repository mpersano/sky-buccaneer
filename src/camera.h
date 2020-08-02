#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    Camera();

    void setEye(const glm::vec3 &eye);
    glm::vec3 eye() const { return m_eye; }

    void setCenter(const glm::vec3 &center);
    glm::vec3 center() const { return m_center; }

    void setUp(const glm::vec3 &up);
    glm::vec3 up() const { return m_up; }

    glm::mat4 viewMatrix() const { return m_viewMatrix; }

private:
    void updateViewMatrix();

    glm::vec3 m_eye;
    glm::vec3 m_center;
    glm::vec3 m_up;
    glm::mat4 m_viewMatrix;
};
