#include "gameobject.h"

#include "entity.h"
#include "world.h"

GameObject::GameObject(World *world, const char *entityPath)
    : m_world(world)
    , m_entity(new Entity)
    , m_position(glm::vec3(0))
    , m_rotation(glm::mat3(1))
{
    m_entity->load(entityPath);
    updateTransformMatrix();
}

GameObject::~GameObject() = default;

void GameObject::setPosition(const glm::vec3 &position)
{
    m_position = position;
    updateTransformMatrix();
}

void GameObject::setRotation(const glm::mat3 &rotation)
{
    m_rotation = rotation;
    updateTransformMatrix();
}

void GameObject::updateTransformMatrix()
{
    const auto t = glm::translate(glm::mat4(1), m_position);
    const auto r = glm::mat4(m_rotation);
    m_transformMatrix = t * r;
}

void GameObject::render(Renderer *renderer) const
{
    m_entity->render(renderer, m_transformMatrix, 0);
}

std::optional<glm::vec3> GameObject::findCollision(const LineSegment &segment) const
{
    return m_entity->findCollision(segment, m_transformMatrix, 0);
}
