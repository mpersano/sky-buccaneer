#include "player.h"

#include "world.h"

#include <glm/gtc/matrix_transform.hpp>

constexpr auto PlayerEntityPath = "assets/meshes/player-ship.w3d";

Player::Player(World *world)
    : GameObject(world, PlayerEntityPath)
{
}

Player::~Player() = default;

void Player::update(float elapsed)
{
    m_fireDelay = std::max(m_fireDelay - elapsed, 0.0f);

    const auto rotate = [this](float angle, const glm::vec3 &axis) {
        const auto r = glm::mat3(glm::rotate(glm::mat4(1), angle, axis));
        setRotation(rotation() * r);
    };

    const auto move = [this](float offset) {
        setPosition(position() + direction() * offset);
    };

    const auto testInput = [inputState = m_world->inputState()](InputState flag) {
        return (inputState & flag) != InputState::None;
    };

    constexpr auto Speed = 5.0f;
    constexpr auto AngularVelocity = 1.5f;

    if (testInput(InputState::Left)) {
        const auto angle = elapsed * AngularVelocity;
        rotate(angle, glm::vec3(0, 1, 0));
    }
    if (testInput(InputState::Right)) {
        const auto angle = elapsed * -AngularVelocity;
        rotate(angle, glm::vec3(0, 1, 0));
    }
    if (testInput(InputState::Up)) {
        const auto angle = elapsed * AngularVelocity;
        rotate(angle, glm::vec3(1, 0, 0));
    }
    if (testInput(InputState::Down)) {
        const auto angle = elapsed * -AngularVelocity;
        rotate(angle, glm::vec3(1, 0, 0));
    }
    if (testInput(InputState::Forward)) {
        const auto offset = elapsed * Speed;
        move(offset);
    }
    if (testInput(InputState::Reverse)) {
        const auto offset = elapsed * -Speed;
        move(offset);
    }
    if (testInput(InputState::Fire)) {
        fireBullet();
    }
}

void Player::fireBullet()
{
    if (m_fireDelay > 0.0f)
        return;

    constexpr auto BulletSpeed = 1.5f;
    constexpr auto FireInterval = 0.2f;
    constexpr auto BulletDuration = 5.0;

    const auto offset = glm::vec3(0.5, -0.5, -2.5);

    const auto bulletPosition = position() + rotation() * offset;
    const auto bulletVelocity = BulletSpeed * direction();

    m_world->spawnBullet(bulletPosition, bulletVelocity, BulletDuration);

    m_fireDelay = FireInterval;
}
