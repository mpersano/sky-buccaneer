#include "renderer.h"

#include "mesh.h"
#include "shadermanager.h"
#include "shaderprogram.h"
#include "shadowbuffer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

Renderer::Renderer(ShaderManager *shaderManager, const Camera *camera)
    : m_shadowBuffer(new GL::ShadowBuffer(1024, 1024))
    , m_shaderManager(shaderManager)
    , m_camera(camera)
{
    m_lightPosition = glm::vec3(7, 4, -4);
}

Renderer::~Renderer() = default;

void Renderer::resize(int width, int height)
{
    m_width = width;
    m_height = height;
}

void Renderer::begin()
{
    m_drawCalls.clear();
}

void Renderer::render(const Mesh *mesh, const glm::mat4 &worldMatrix)
{
    m_drawCalls.push_back({ worldMatrix, mesh });
}

void Renderer::end()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);

    // render to shadow buffer

    m_shadowBuffer->bind();
    glViewport(0, 0, m_shadowBuffer->width(), m_shadowBuffer->height());

    glClear(GL_DEPTH_BUFFER_BIT);

    const auto HalfWidth = 20.0f;
    const auto HalfHeight = 20.0f;
    const auto ZNear = 1.0f;
    const auto ZFar = 22.5f;
    const auto lightProjection =
            // glm::ortho(-HalfWidth, HalfWidth, -HalfHeight, HalfHeight, ZNear, ZFar);
            glm::perspective(glm::radians(45.0f), static_cast<float>(m_shadowBuffer->width()) / m_shadowBuffer->height(), 1.0f, 50.f);
    const auto lightView = glm::lookAt(m_lightPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    m_shaderManager->useProgram(ShaderManager::Shadow);
    m_shaderManager->setUniform(ShaderManager::ViewMatrix, lightView);
    m_shaderManager->setUniform(ShaderManager::ProjectionMatrix, lightProjection);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(4, 4);

    for (const auto &drawCall : m_drawCalls) {
        m_shaderManager->setUniform(ShaderManager::ModelMatrix, drawCall.worldMatrix);
        drawCall.mesh->render();
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    m_shadowBuffer->unbind();

    // render scene

    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderManager->useProgram(ShaderManager::Phong);
    m_shaderManager->setUniform(ShaderManager::LightPosition, m_lightPosition);
    m_shaderManager->setUniform(ShaderManager::EyePosition, m_camera->eye());
    m_shaderManager->setUniform(ShaderManager::ShadowMapTexture, 0);
    m_shaderManager->setUniform(ShaderManager::ProjectionMatrix, m_camera->projectionMatrix());
    m_shaderManager->setUniform(ShaderManager::ViewMatrix, m_camera->viewMatrix());
    m_shaderManager->setUniform(ShaderManager::LightViewProjection, lightProjection * lightView);

    m_shadowBuffer->bindTexture();

    for (const auto &drawCall : m_drawCalls) {
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(drawCall.worldMatrix)));
        m_shaderManager->setUniform(ShaderManager::ModelMatrix, drawCall.worldMatrix);
        m_shaderManager->setUniform(ShaderManager::NormalMatrix, normalMatrix);
        drawCall.mesh->render();
    }
}
