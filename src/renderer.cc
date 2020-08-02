#include "renderer.h"

#include "mesh.h"
#include "shaderprogram.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

Renderer::Renderer()
    : m_shaderProgram(new GL::ShaderProgram)
{
    m_shaderProgram->addShader(GL_VERTEX_SHADER, "assets/shaders/simple.vert");
    m_shaderProgram->addShader(GL_FRAGMENT_SHADER, "assets/shaders/simple.frag");
    m_shaderProgram->link();

    // XXX
    const auto eye = glm::vec3(0, 0, 4);
    const auto center = glm::vec3(0, 0, 0);
    const auto up = glm::vec3(0, 1, 0);
    m_viewMatrix = glm::lookAt(eye, center, up);
}

Renderer::~Renderer() = default;

void Renderer::resize(int width, int height)
{
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.f);
}

void Renderer::render(const Entity *entity, const glm::mat4 &worldMatrix)
{
    m_shaderProgram->bind();
    m_shaderProgram->setUniform("lightPosition", glm::vec3(0, 0, 12));
    m_shaderProgram->setUniform("color", glm::vec3(1));
    for (const auto *node : entity->m_rootNodes) {
        render(node, worldMatrix);
    }
}

void Renderer::render(const Entity::Node *node, const glm::mat4 &parentWorldMatrix)
{
    const auto localMatrix = node->transform.matrix();
    const auto worldMatrix = parentWorldMatrix * localMatrix;
    if (auto mesh = node->mesh.get()) {
        const auto mvp = m_projectionMatrix * m_viewMatrix * worldMatrix;
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(m_viewMatrix * worldMatrix)));
        m_shaderProgram->setUniform("mvp", mvp);
        m_shaderProgram->setUniform("modelMatrix", worldMatrix);
        m_shaderProgram->setUniform("normalMatrix", normalMatrix);
        mesh->render();
    }
    for (const auto *child : node->children) {
        render(child, worldMatrix);
    }
}
