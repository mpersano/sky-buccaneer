#pragma once

#include "geometryutils.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Mesh;
class Material;
class Renderer;
class Octree;
class DataStream;

#define DRAW_RAW_LEVEL_MESHES 0

class Level
{
public:
    Level();
    ~Level();

    bool load(const char *path);
    void render(Renderer *renderer) const;
    std::optional<glm::vec3> findCollision(const LineSegment &segment) const;

private:
    bool load(DataStream &ds);
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
