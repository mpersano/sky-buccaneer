#include "renderer.h"

#include "mesh.h"
#include "shaderprogram.h"
#include "shadowbuffer.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>

Renderer::Renderer()
    : m_shaderPhong(new GL::ShaderProgram)
    , m_shaderShadow(new GL::ShaderProgram)
    , m_shadowBuffer(new GL::ShadowBuffer(1024, 1024))
{
    // XXX move these to shader manager

    m_shaderPhong->addShader(GL_VERTEX_SHADER, "assets/shaders/phong.vert");
    m_shaderPhong->addShader(GL_FRAGMENT_SHADER, "assets/shaders/phong.frag");
    m_shaderPhong->link();

    m_shaderShadow->addShader(GL_VERTEX_SHADER, "assets/shaders/shadow.vert");
    m_shaderShadow->addShader(GL_FRAGMENT_SHADER, "assets/shaders/shadow.frag");
    m_shaderShadow->link();

    m_camera.setEye(glm::vec3(8, 0, 0));
    m_lightPosition = glm::vec3(7, 4, -4);
}

Renderer::~Renderer() = default;

void Renderer::resize(int width, int height)
{
    m_width = width;
    m_height = height;
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.f);
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
    const auto lightView = glm::lookAt(m_lightPosition, m_camera.center(), glm::vec3(0, 1, 0));

    m_shaderShadow->bind();
    m_shaderShadow->setUniform("viewMatrix", lightView);
    m_shaderShadow->setUniform("projectionMatrix", lightProjection);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(4, 4);

    for (const auto &drawCall : m_drawCalls) {
        m_shaderShadow->setUniform("modelMatrix", drawCall.worldMatrix);
        drawCall.mesh->render();
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    m_shadowBuffer->unbind();

    // render scene

    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderPhong->bind();
    m_shaderPhong->setUniform("lightPosition", m_lightPosition);
    m_shaderPhong->setUniform("eyePosition", m_camera.eye());
    m_shaderPhong->setUniform("shadowMapTexture", 0);
    m_shaderPhong->setUniform("projectionMatrix", m_projectionMatrix);
    m_shaderPhong->setUniform("viewMatrix", m_camera.viewMatrix());
    m_shaderPhong->setUniform("lightViewProjection", lightProjection * lightView);

    m_shadowBuffer->bindTexture();

    for (const auto &drawCall : m_drawCalls) {
        const auto normalMatrix = glm::transpose(glm::inverse(glm::mat3(drawCall.worldMatrix)));
        m_shaderPhong->setUniform("modelMatrix", drawCall.worldMatrix);
        m_shaderPhong->setUniform("normalMatrix", normalMatrix);
        drawCall.mesh->render();
    }
}
