#include "gameobject.h"

class Player : public GameObject
{
public:
    explicit Player(World *world);
    ~Player() override;

    void update(float elapsed) override;

private:
    void fireBullet();

    float m_fireDelay = 0.0f;
};
