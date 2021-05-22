#include "world.h"

#include "camera.h"
#include "entity.h"
#include "level.h"
#include "materialcache.h"
#include "mesh.h"
#include "renderer.h"
#include "shadermanager.h"

#include <algorithm>
#include <iostream>

#include <GL/glew.h>

#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

namespace {
struct BulletState {
    glm::vec3 position;
    glm::vec3 velocity;
};

constexpr const auto MaxBullets = 200;

std::unique_ptr<Mesh> makeBulletMesh()
{
    auto mesh = std::make_unique<Mesh>(GL_POINTS);

    static const std::vector<Mesh::VertexAttribute> attributes = {
        { 3, GL_FLOAT, offsetof(BulletState, position) },
        { 3, GL_FLOAT, offsetof(BulletState, velocity) },
    };

    mesh->setVertexCount(MaxBullets);
    mesh->setVertexSize(sizeof(BulletState));
    mesh->setVertexAttributes(attributes);
    mesh->initialize();

    return mesh;
}

const Material *bulletMaterial()
{
    static const auto material = Material(ShaderManager::Program::Billboard);
    return &material;
}
} // namespace

World::World()
    : m_shaderManager(new ShaderManager)
    , m_materialCache(new MaterialCache)
    , m_camera(new Camera)
    , m_renderer(new Renderer(m_shaderManager.get(), m_camera.get()))
    , m_level(new Level)
    , m_playerEntity(new Entity)
    , m_explosionEntity(new Entity)
    , m_bulletsMesh(makeBulletMesh())
{
    m_playerState.position = glm::vec3(0, 0, 0);
    m_playerState.rotation = glm::mat3(1);

    m_level->load("assets/meshes/level.z3d", m_materialCache.get());
    m_playerEntity->load("assets/meshes/player-ship.w3d", m_materialCache.get());
    m_explosionEntity->load("assets/meshes/fireball.w3d", m_materialCache.get());

    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

World::~World() = default;

void World::resize(int width, int height)
{
    m_camera->setAspectRatio(static_cast<float>(width) / height);
    m_renderer->resize(width, height);
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
    m_level->render(m_renderer.get());
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
    if (!m_bullets.empty()) {
        std::vector<BulletState> bulletData;
        std::transform(m_bullets.begin(), m_bullets.end(), std::back_inserter(bulletData), [](const Bullet &bullet) -> BulletState {
            return { bullet.position, bullet.velocity };
        });
        m_bulletsMesh->setVertexCount(m_bullets.size());
        m_bulletsMesh->setVertexData(bulletData.data());
        m_renderer->render(m_bulletsMesh.get(), bulletMaterial(), glm::mat4(1));
    }
    m_renderer->end();
}

void World::update(InputState inputState, float elapsed)
{
    updateBullets(elapsed);
    updateExplosions(elapsed);
    updatePlayer(inputState, elapsed);
}

void World::updateBullets(float elapsed)
{
    const auto updateBullet = [this, elapsed](Bullet &bullet) {
        bullet.lifetime -= elapsed;
        if (bullet.lifetime < 0.0)
            return false;
        const auto d = glm::normalize(bullet.velocity);
        constexpr auto BulletLength = 2.0f;
        const auto p0 = bullet.position - 0.5f * BulletLength * d;
        const auto p1 = bullet.position + 0.5f * BulletLength * d;
        if (auto collisionPosition = m_level->findCollision(p0, p1)) {
            spawnExplosion(*collisionPosition);
            return false;
        }
        bullet.position += bullet.velocity;
        return true;
    };
    auto it = m_bullets.begin();
    while (it != m_bullets.end()) {
        auto &bullet = *it;
        if (!updateBullet(*it))
            it = m_bullets.erase(it);
        else
            ++it;
    }
}

void World::updateExplosions(float elapsed)
{
    auto it = m_explosions.begin();
    while (it != m_explosions.end()) {
        auto &explosion = *it;
        explosion.lifetime -= elapsed;
        if (explosion.lifetime < 0.0) {
            it = m_explosions.erase(it);
            continue;
        }
        ++it;
    }
}

void World::updatePlayer(InputState inputState, float elapsed)
{
    m_fireDelay = std::max(m_fireDelay - elapsed, 0.0f);

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
    if (testFlag(InputState::Fire)) {
        fireBullet();
    }
    if (testFlag(InputState::ToggleView) && (m_prevInputState & InputState::ToggleView) == InputState::None) {
        m_cameraMode = m_cameraMode == CameraMode::FirstPerson ? CameraMode::ThirdPerson : CameraMode::FirstPerson;
    }
    m_prevInputState = inputState;
}

void World::fireBullet()
{
    if (m_fireDelay > 0.0f || m_bullets.size() >= MaxBullets)
        return;

    constexpr auto BulletSpeed = 1.5f;
    constexpr auto FireInterval = 0.2f;
    constexpr auto BulletDuration = 5.0;

    const auto offset = glm::vec3(0.5, -0.5, -2.5);

    Bullet bullet;
    bullet.position = m_playerState.position + m_playerState.rotation * offset;
    bullet.velocity = BulletSpeed * m_playerState.rotation[2];
    bullet.lifetime = BulletDuration;
    m_bullets.push_back(bullet);

    m_fireDelay = FireInterval;
}

void World::spawnExplosion(const glm::vec3 &position)
{
    constexpr auto ExplosionDuration = 1.0;
    m_explosions.push_back({ position, ExplosionDuration });
}
