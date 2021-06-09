#pragma once

#include "inputstate.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class ShaderManager;
class Renderer;
class Entity;
class Mesh;
class Level;
class Camera;
class Foe;
class Player;

class World
{
public:
    World();
    ~World();

    ShaderManager *shaderManager() { return m_shaderManager.get(); }
    InputState inputState() { return m_inputState; }

    void resize(int width, int height);
    void update(InputState inputState, float elapsed);
    void render() const;

    void spawnBullet(const glm::vec3 &position, const glm::vec3 &velocity, float duration);

private:
    void updateBullets(float elapsed);
    void updateExplosions(float elapsed);
    void spawnExplosion(const glm::vec3 &center);

    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Player> m_player;
    std::unique_ptr<Foe> m_foe;
    std::unique_ptr<Level> m_level;
    std::unique_ptr<Entity> m_explosionEntity;
    std::unique_ptr<Mesh> m_bulletsMesh;
    enum class CameraMode {
        FirstPerson,
        ThirdPerson
    } m_cameraMode = CameraMode::ThirdPerson;
    InputState m_inputState;
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
};
