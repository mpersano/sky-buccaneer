#include "level.h"

#include "datastream.h"
#include "material.h"
#include "materialcache.h"
#include "mesh.h"
#include "octree.h"
#include "panic.h"
#include "renderer.h"

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <limits>

Level::Level()
    : m_octree(new Octree)
{
}

Level::~Level() = default;

void Level::load(const char *path, MaterialCache *materialCache)
{
    std::vector<Face> faces;

    DataStream ds(path);
    if (!ds) {
        panic("failed to open %s\n", path);
    }

    uint32_t meshCount;
    ds >> meshCount;

    std::cout << "Reading " << meshCount << " meshes\n";
    for (int i = 0; i < meshCount; ++i) {
        MaterialKey materialKey;
        ds >> materialKey;
        const auto *material = materialCache->cachedMaterial(materialKey);

        std::vector<MeshVertex> vertices;
        ds >> vertices;

        uint32_t faceCount;
        ds >> faceCount;

        std::vector<unsigned> indices;

        for (int i = 0; i < faceCount; ++i) {
            uint8_t faceIndexCount;
            ds >> faceIndexCount;

            std::vector<uint32_t> faceIndices;
            faceIndices.reserve(faceIndexCount);
            for (int j = 0; j < faceIndexCount; ++j) {
                uint32_t index;
                ds >> index;
                faceIndices.push_back(index);
            }

            Face face;
            face.material = material;
            for (const auto &index : faceIndices) {
                const auto &v = vertices[index];
                face.vertices.push_back({ v.position, v.normal, v.texcoord });
            }
            faces.push_back(face);

            for (int j = 1; j < faceIndexCount - 1; ++j) {
                indices.push_back(faceIndices[0]);
                indices.push_back(faceIndices[j]);
                indices.push_back(faceIndices[j + 1]);
            }

#if DRAW_RAW_LEVEL_MESHES
            for (int j = 1; j < faceIndexCount - 1; ++j) {
                const auto &v0 = vertices[faceIndices[0]];
                const auto &v1 = vertices[faceIndices[j]];
                const auto &v2 = vertices[faceIndices[j + 1]];
                m_triangles.push_back({ v0.position, v1.position, v2.position });
            }
#endif
        }

        std::cout << "**** mesh: " << vertices.size() << ' ' << indices.size() << '\n';
        std::cout << glm::to_string(vertices[0].position) << ' ' << glm::to_string(vertices[1].position) << '\n';

#if DRAW_RAW_LEVEL_MESHES
        auto mesh = makeMesh(GL_TRIANGLES, vertices, indices);
        m_meshes.push_back({ std::move(mesh), material });
#endif
    }

    m_octree->initialize(faces);
}

void Level::render(Renderer *renderer) const
{
#if DRAW_RAW_LEVEL_MESHES
    for (const auto &m : m_meshes) {
        renderer->render(m.mesh.get(), m.material, glm::mat4(1));
    }
#else
    m_octree->render(renderer, glm::mat4(1));
#endif
}

std::optional<glm::vec3> Level::findCollision(const glm::vec3 &p0, const glm::vec3 &p1) const
{
#if DRAW_RAW_LEVEL_MESHES
    const auto ray = Ray { p0, p1 - p0 };
    std::optional<glm::vec3> collision;
    auto collisionT = std::numeric_limits<float>::max();
    for (const auto &triangle : m_triangles) {
        if (auto ot = rayTriangleIntersection(ray, triangle)) {
            const auto t = *ot;
            if (t < collisionT) {
                collisionT = t;
                collision = p0 + t * (p1 - p0);
            }
        }
    }
    return collision;
#endif
    return {};
}
