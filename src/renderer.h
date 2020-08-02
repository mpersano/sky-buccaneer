#pragma once

#include "entity.h"

#include <glm/glm.hpp>

#include <memory>

namespace GL {
class ShaderProgram;
}

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void resize(int width, int height);
    void render(const Entity *entity, const glm::mat4 &worldMatrix);

private:
    void render(const Entity::Node *node, const glm::mat4 &worldMatrix);

    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;
    std::unique_ptr<GL::ShaderProgram> m_shaderProgram;
};
