#pragma once

#include <glm/glm.hpp>

#include <memory>

class Mesh;

namespace GL {
class ShaderProgram;
}

class World
{
public:
    World();
    ~World();

    void resize(int width, int height);
    void update(double elapsed);
    void render() const;

private:
    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;
    std::unique_ptr<GL::ShaderProgram> m_shaderProgram;
    std::unique_ptr<Mesh> m_mesh;
    double m_time = 0.0;
};
