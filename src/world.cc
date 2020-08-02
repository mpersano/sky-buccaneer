#include "world.h"

#include "panic.h"
#include "entity.h"

#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>

#include <fstream>

World::World()
    : m_entity(new Entity)
{
    m_entity->load("assets/meshes/scene.bin");

    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

World::~World() = default;

void World::resize(int width, int height)
{
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.f);

    const auto eye = glm::vec3(0, 0, 3);
    const auto center = glm::vec3(0, 0, 0);
    const auto up = glm::vec3(0, 1, 0);
    m_viewMatrix = glm::lookAt(eye, center, up);

    glViewport(0, 0, width, height);
}

void World::render() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto modelMatrix = glm::rotate(glm::mat4(1), 3.0f * static_cast<float>(m_time), glm::vec3(-1, -0.5, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f));
    const auto mvp = m_projectionMatrix * m_viewMatrix * modelMatrix;

    m_entity->render(mvp);
}

void World::update(double elapsed)
{
    m_time += elapsed;
}
