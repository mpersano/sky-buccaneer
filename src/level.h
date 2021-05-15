#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Mesh;
class Material;
class Renderer;
class MaterialCache;
class Octree;

#define DRAW_RAW_LEVEL_MESHES 1

class Level
{
public:
    Level();
    ~Level();

    void load(const char *filepath, MaterialCache *materialCache);
    void render(Renderer *renderer, const glm::mat4 &worldMatrix) const;

private:
#if DRAW_RAW_LEVEL_MESHES
    struct MeshMaterial {
        std::unique_ptr<Mesh> mesh;
        const Material *material;
    };
    std::vector<MeshMaterial> m_meshes;
#endif
    std::unique_ptr<Octree> m_octree;
};
