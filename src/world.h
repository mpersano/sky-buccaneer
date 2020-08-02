#pragma once

#include <glm/glm.hpp>

#include <memory>

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
    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;
    std::unique_ptr<Entity> m_entity;
    double m_time = 0.0;
};
