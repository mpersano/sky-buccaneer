#include "world.h"

#include "entity.h"
#include "panic.h"
#include "renderer.h"

#include <GL/glew.h>

#include <fstream>

World::World()
    : m_renderer(new Renderer)
    , m_entity(new Entity)
{
    m_entity->load("assets/meshes/scene.bin");

    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

World::~World() = default;

void World::resize(int width, int height)
{
    m_renderer->resize(width, height);
    glViewport(0, 0, width, height);
}

void World::render() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto modelMatrix = glm::rotate(glm::mat4(1), 3.0f * static_cast<float>(m_time), glm::vec3(-1, -0.5, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f));
    m_renderer->render(m_entity.get(), modelMatrix);
}

void World::update(double elapsed)
{
    m_time += elapsed;
}
