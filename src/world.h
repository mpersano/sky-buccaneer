#pragma once

#include "inputstate.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class ShaderManager;
class MaterialCache;
class Renderer;
class Entity;
class Mesh;
class Level;
class Camera;

class World
{
public:
    World();
    ~World();

    void resize(int width, int height);
    void update(InputState inputState, float elapsed);
    void render() const;

private:
    void updatePlayer(InputState inputState, float elapsed);
    void updateBullets(float elapsed);
    void updateExplosions(float elapsed);
    void fireBullet();
    void spawnExplosion(const glm::vec3 &center);

    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<MaterialCache> m_materialCache;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Renderer> m_renderer;
    struct Player {
        glm::vec3 position;
        glm::mat3 rotation;
    };
    Player m_playerState;
    std::unique_ptr<Level> m_level;
    std::unique_ptr<Entity> m_playerEntity;
    std::unique_ptr<Entity> m_explosionEntity;
    std::unique_ptr<Mesh> m_bulletsMesh;
    enum class CameraMode {
        FirstPerson,
        ThirdPerson
    } m_cameraMode = CameraMode::ThirdPerson;
    InputState m_prevInputState;
    struct Explosion {
        glm::vec3 position;
        float lifetime;
    };
    std::vector<Explosion> m_explosions;
    struct Bullet {
        glm::vec3 position;
        glm::vec3 velocity;
        float lifetime;
    };
    std::vector<Bullet> m_bullets;
    float m_fireDelay = 0.0f;
};
