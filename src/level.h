#pragma once

#include "geometryutils.h"

#include <glm/glm.hpp>

#include <memory>
#include <optional>
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

    void load(const char *path, MaterialCache *materialCache);
    void render(Renderer *renderer, const glm::mat4 &worldMatrix) const;
    std::optional<glm::vec3> findCollision(const glm::vec3 &p0, const glm::vec3 &p1) const;

private:
#if DRAW_RAW_LEVEL_MESHES
    struct MeshMaterial {
        std::unique_ptr<Mesh> mesh;
        const Material *material;
    };
    std::vector<MeshMaterial> m_meshes;
    std::vector<Triangle> m_triangles;
#endif
    std::unique_ptr<Octree> m_octree;
};
