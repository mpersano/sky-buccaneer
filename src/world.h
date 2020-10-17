#pragma once

#include "inputstate.h"

#include <glm/glm.hpp>

#include <memory>

class ShaderManager;
class MaterialCache;
class Renderer;
class Entity;
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
    Player m_player;
    std::unique_ptr<Entity> m_entity;
    double m_time = 0.0;
};
