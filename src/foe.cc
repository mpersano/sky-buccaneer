#include "foe.h"

constexpr auto FoeEntityPath = "assets/meshes/player-ship.w3d";

Foe::Foe(World *world)
    : GameObject(world, FoeEntityPath)
{
}

Foe::~Foe() = default;

void Foe::update(float)
{
}
