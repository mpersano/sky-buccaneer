#include "world.h"

#include "camera.h"
#include "entity.h"
#include "foe.h"
#include "level.h"
#include "material.h"
#include "mesh.h"
#include "player.h"
#include "renderer.h"
#include "shadermanager.h"

#include <algorithm>

#include <GL/glew.h>

#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

namespace {
struct BulletState {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec2 size;
};

constexpr const auto MaxBullets = 200;
constexpr const auto BulletSize = glm::vec2(0.1, 2);

std::unique_ptr<Mesh> makeBulletMesh()
{
    auto mesh = std::make_unique<Mesh>(GL_POINTS);

    static const std::vector<Mesh::VertexAttribute> attributes = {
        { 3, GL_FLOAT, offsetof(BulletState, position) },
        { 3, GL_FLOAT, offsetof(BulletState, velocity) },
        { 2, GL_FLOAT, offsetof(BulletState, size) }
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
    , m_camera(new Camera)
    , m_renderer(new Renderer(m_shaderManager.get(), m_camera.get()))
    , m_level(new Level)
    , m_player(new Player(this))
    , m_foe(new Foe(this))
    , m_explosionEntity(new Entity)
    , m_bulletsMesh(makeBulletMesh())
{
    m_level->load("assets/meshes/level.z3d");
    m_explosionEntity->load("assets/meshes/fireball.w3d");

    m_foe->setPosition(glm::vec3(8.0, 0.0, 0.0));
    m_foe->setRotation(glm::mat3(glm::rotate(glm::mat4(1), .5f, glm::normalize(glm::vec3(1.0f)))));

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
    const auto playerPosition = m_player->position();
    const auto playerRotation = m_player->rotation();
    if (m_cameraMode == CameraMode::FirstPerson) {
        const auto playerDir = playerRotation[2];
        const auto playerUp = playerRotation[1];
        m_camera->setEye(playerPosition);
        m_camera->setCenter(playerPosition + playerDir);
        m_camera->setUp(playerUp);
    } else {
        m_camera->setEye(glm::vec3(0, 0, 0));
        m_camera->setCenter(playerPosition);
        m_camera->setUp(glm::vec3(0, 1, 0));
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_renderer->begin();
    m_level->render(m_renderer.get());
    if (m_cameraMode == CameraMode::ThirdPerson) {
        m_player->render(m_renderer.get());
    }
    for (const auto &explosion : m_explosions) {
        const auto t = glm::translate(glm::mat4(1), explosion.position);
        const auto s = glm::scale(glm::mat4(1), glm::vec3(.5));
        m_explosionEntity->render(m_renderer.get(), t * s, 0);
    }
    if (!m_bullets.empty()) {
        std::vector<BulletState> bulletData;
        std::transform(m_bullets.begin(), m_bullets.end(), std::back_inserter(bulletData), [](const Bullet &bullet) -> BulletState {
            return { bullet.position, bullet.velocity, BulletSize };
        });
        m_bulletsMesh->setVertexCount(m_bullets.size());
        m_bulletsMesh->setVertexData(bulletData.data());
        m_renderer->render(m_bulletsMesh.get(), bulletMaterial(), glm::mat4(1));
    }
    m_foe->render(m_renderer.get());
    m_renderer->end();
}

void World::update(InputState inputState, float elapsed)
{
    const auto prevInputState = m_inputState;
    m_inputState = inputState;

    const auto toggleViewPressed = [](InputState inputState) {
        return (inputState & InputState::ToggleView) != InputState::None;
    };
    if (toggleViewPressed(inputState) && !toggleViewPressed(prevInputState))
        m_cameraMode = m_cameraMode == CameraMode::FirstPerson ? CameraMode::ThirdPerson : CameraMode::FirstPerson;

    updateBullets(elapsed);
    updateExplosions(elapsed);
    m_player->update(elapsed);
}

void World::updateBullets(float elapsed)
{
    const auto updateBullet = [this, elapsed](Bullet &bullet) {
        bullet.lifetime -= elapsed;
        if (bullet.lifetime < 0.0)
            return false;
        const auto d = glm::normalize(bullet.velocity);
        const auto p0 = bullet.position - 0.5f * BulletSize.y * d;
        const auto p1 = bullet.position + 0.5f * BulletSize.y * d;
        const auto segment = LineSegment { p0, p1 };
        const auto collisionPosition = [this, &segment] {
            if (auto position = m_level->findCollision(segment))
                return position;
            return m_foe->findCollision(segment);
        }();
        if (collisionPosition) {
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

void World::spawnBullet(const glm::vec3 &position, const glm::vec3 &velocity, float duration)
{
    if (m_bullets.size() >= MaxBullets)
        return;
    m_bullets.push_back({ position, velocity, duration });
}

void World::spawnExplosion(const glm::vec3 &position)
{
    constexpr auto ExplosionDuration = 1.0;
    m_explosions.push_back({ position, ExplosionDuration });
}
