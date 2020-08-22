#include "world.h"

#include "camera.h"
#include "entity.h"
#include "renderer.h"
#include "shadermanager.h"

#include <iostream>

#include <GL/glew.h>

#include <glm/gtx/string_cast.hpp>

World::World()
    : m_shaderManager(new ShaderManager)
    , m_camera(new Camera)
    , m_renderer(new Renderer(m_shaderManager.get(), m_camera.get()))
    , m_entity(new Entity)
{
    m_player.position = glm::vec3(8, 0, 0);
    m_player.rotation = glm::rotate(glm::mat4(1), -0.5f * glm::pi<float>(), glm::vec3(0, 1, 0));

    m_entity->load("assets/meshes/scene.bin");
    m_entity->setActiveAction("Empty", "TestAction");

    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

World::~World() = default;

void World::resize(int width, int height)
{
    m_camera->setAspectRatio(static_cast<float>(width) / height);
    m_renderer->resize(width, height);
    glViewport(0, 0, width, height);
}

void World::render() const
{
    const auto playerDir = m_player.rotation[2];
    const auto playerUp = m_player.rotation[1];

    m_camera->setEye(m_player.position);
    m_camera->setCenter(m_player.position + playerDir);
    m_camera->setUp(playerUp);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_renderer->begin();
    m_entity->render(m_renderer.get(), glm::mat4(1), std::fmod(6.0f * m_time, 21.0f));
    m_renderer->end();
}

void World::update(InputState inputState, double elapsed)
{
    m_time += elapsed;

    const auto rotatePlayer = [this](float angle, const glm::vec3 &axis) {
        const auto r = glm::mat3(glm::rotate(glm::mat4(1), angle, axis));
        m_player.rotation *= r;
    };

    const auto movePlayer = [this](float offset) {
        const auto d = m_player.rotation[2];
        m_player.position += d * offset;
    };

    const auto testFlag = [inputState](InputState flag) {
        return (inputState & flag) != InputState::None;
    };

    constexpr auto Speed = 5.0f;
    constexpr auto AngularVelocity = 1.5f;

    if (testFlag(InputState::Left)) {
        const auto angle = elapsed * AngularVelocity;
        rotatePlayer(angle, glm::vec3(0, 1, 0));
    }
    if (testFlag(InputState::Right)) {
        const auto angle = elapsed * -AngularVelocity;
        rotatePlayer(angle, glm::vec3(0, 1, 0));
    }
    if (testFlag(InputState::Up)) {
        const auto angle = elapsed * AngularVelocity;
        rotatePlayer(angle, glm::vec3(1, 0, 0));
    }
    if (testFlag(InputState::Down)) {
        const auto angle = elapsed * -AngularVelocity;
        rotatePlayer(angle, glm::vec3(1, 0, 0));
    }
    if (testFlag(InputState::Forward)) {
        const auto offset = elapsed * Speed;
        movePlayer(offset);
    }
    if (testFlag(InputState::Reverse)) {
        const auto offset = elapsed * -Speed;
        movePlayer(offset);
    }
}
