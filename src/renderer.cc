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
    const auto eye = glm::vec3(8, 0, 0);
    const auto center = glm::vec3(0, 0, 0);
    const auto up = glm::vec3(0, 1, 0);
    m_viewMatrix = glm::lookAt(eye, center, up);
}

Renderer::~Renderer() = default;

void Renderer::resize(int width, int height)
{
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.f);
}

void Renderer::begin()
{
}

void Renderer::render(const Mesh *mesh, const glm::mat4 &worldMatrix)
{
    m_shaderProgram->bind();
    m_shaderProgram->setUniform("lightPosition", glm::vec3(8, 0, 0));
    m_shaderProgram->setUniform("color", glm::vec3(1));

    const auto mvp = m_projectionMatrix * m_viewMatrix * worldMatrix;
    const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(worldMatrix)));

    m_shaderProgram->setUniform("mvp", mvp);
    m_shaderProgram->setUniform("modelMatrix", worldMatrix);
    m_shaderProgram->setUniform("normalMatrix", normalMatrix);
    mesh->render();
}

void Renderer::end()
{
}
