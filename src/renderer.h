#pragma once

#include <glm/glm.hpp>

#include <memory>

class Mesh;

namespace GL {
class ShaderProgram;
}

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void resize(int width, int height);

    void begin();
    void render(const Mesh *mesh, const glm::mat4 &worldMatrix);
    void end();

private:
    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;
    std::unique_ptr<GL::ShaderProgram> m_shaderProgram;
};
