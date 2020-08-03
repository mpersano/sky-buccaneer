#pragma once

#include <glm/glm.hpp>

#include <memory>

class ShaderManager;
class Renderer;
class Entity;

class World
{
public:
    World();
    ~World();

    void resize(int width, int height);
    void update(double elapsed);
    void render() const;

private:
    std::unique_ptr<ShaderManager> m_shaderManager;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Entity> m_entity;
    double m_time = 0.0;
};
