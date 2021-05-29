#include "gameobject.h"

class Foe : public GameObject
{
public:
    explicit Foe(World *world);
    ~Foe() override;

    void update(float elapsed) override;
};
