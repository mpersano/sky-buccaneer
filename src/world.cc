#include "world.h"

#include "camera.h"
#include "entity.h"
#include "level.h"
#include "materialcache.h"
#include "renderer.h"
#include "shadermanager.h"

#include <algorithm>
#include <iostream>

#include <GL/glew.h>

#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

World::World()
    : m_shaderManager(new ShaderManager)
    , m_materialCache(new MaterialCache)
    , m_camera(new Camera)
    , m_renderer(new Renderer(m_shaderManager.get(), m_camera.get()))
    , m_level(new Level)
    , m_playerEntity(new Entity)
    , m_explosionEntity(new Entity)
{
    m_playerState.position = glm::vec3(0, 0, 0);
    m_playerState.rotation = glm::mat3(1);

    m_level->load("assets/meshes/level.z3d", m_materialCache.get());
    m_playerEntity->load("assets/meshes/player-ship.w3d", m_materialCache.get());
    m_explosionEntity->load("assets/meshes/fireball.w3d", m_materialCache.get());

    std::generate_n(std::back_inserter(m_explosions), 20, []() -> Explosion {
        return { glm::linearRand(glm::vec3(-10), glm::vec3(10)) };
    });

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
    if (m_cameraMode == CameraMode::FirstPerson) {
        const auto playerDir = m_playerState.rotation[2];
        const auto playerUp = m_playerState.rotation[1];
        m_camera->setEye(m_playerState.position);
        m_camera->setCenter(m_playerState.position + playerDir);
        m_camera->setUp(playerUp);
    } else {
        m_camera->setEye(glm::vec3(0, 0, 0));
        m_camera->setCenter(m_playerState.position);
        m_camera->setUp(glm::vec3(0, 1, 0));
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_renderer->begin();
    m_level->render(m_renderer.get(), glm::mat4(1));
    if (m_cameraMode == CameraMode::ThirdPerson) {
        const auto t = glm::translate(glm::mat4(1), m_playerState.position);
        const auto r = glm::mat4(m_playerState.rotation);
        const auto playerWorldMatrix = t * r;
        m_playerEntity->render(m_renderer.get(), playerWorldMatrix, 0);
    }
    for (const auto &explosion : m_explosions) {
        const auto t = glm::translate(glm::mat4(1), explosion.position);
        const auto s = glm::scale(glm::mat4(1), glm::vec3(.5));
        m_explosionEntity->render(m_renderer.get(), t * s, 0);
    }
    m_renderer->end();
}

void World::update(InputState inputState, double elapsed)
{
    m_time += elapsed;

    const auto rotatePlayer = [this](float angle, const glm::vec3 &axis) {
        const auto r = glm::mat3(glm::rotate(glm::mat4(1), angle, axis));
        m_playerState.rotation *= r;
    };

    const auto movePlayer = [this](float offset) {
        const auto d = m_playerState.rotation[2];
        m_playerState.position += d * offset;
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
    if (testFlag(InputState::ToggleView) && (m_prevInputState & InputState::ToggleView) == InputState::None) {
        m_cameraMode = m_cameraMode == CameraMode::FirstPerson ? CameraMode::ThirdPerson : CameraMode::FirstPerson;
    }
    m_prevInputState = inputState;
}
