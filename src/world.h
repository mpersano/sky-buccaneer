#pragma once

#include "inputstate.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class ShaderManager;
class MaterialCache;
class Renderer;
class Entity;
class Level;
class Camera;

class World
{
public:
    World();
    ~World();

    void resize(int width, int height);
    void update(InputState inputState, double elapsed);
    void render() const;

private:
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
    double m_time = 0.0;
    enum class CameraMode {
        FirstPerson,
        ThirdPerson
    } m_cameraMode = CameraMode::ThirdPerson;
    InputState m_prevInputState;
    struct Explosion {
        glm::vec3 position;
    };
    std::vector<Explosion> m_explosions;
};
