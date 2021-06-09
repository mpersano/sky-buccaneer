#pragma once

#include "geometryutils.h"

#include <glm/glm.hpp>

#include <memory>

class Entity;
class Renderer;
class World;

class GameObject
{
public:
    explicit GameObject(World *world, const char *entityPath);
    virtual ~GameObject();

    void setPosition(const glm::vec3 &position);
    glm::vec3 position() const { return m_position; }

    void setRotation(const glm::mat3 &rotation);
    glm::mat3 rotation() const { return m_rotation; }

    glm::vec3 direction() const { return m_rotation[2]; }

    glm::mat4 transformMatrix() const { return m_transformMatrix; }

    void render(Renderer *renderer) const;

    std::optional<glm::vec3> findCollision(const LineSegment &segment) const;

    virtual void update(float elapsed) = 0;

protected:
    World *m_world;

private:
    void updateTransformMatrix();

    std::unique_ptr<Entity> m_entity;
    glm::vec3 m_position;
    glm::mat3 m_rotation;
    glm::mat4 m_transformMatrix;
};
