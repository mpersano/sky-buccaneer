#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Mesh;
class Renderer;
class MaterialCache;

class Level
{
public:
    Level();
    ~Level();

    void load(const char *filepath, MaterialCache *materialCache);
    void render(Renderer *renderer, const glm::mat4 &worldMatrix) const;

private:
    std::vector<std::unique_ptr<Mesh>> m_meshes;
};
