#include "level.h"

#include "datastream.h"
#include "material.h"
#include "materialcache.h"
#include "mesh.h"
#include "panic.h"
#include "renderer.h"

#include <iostream>

namespace {

DataStream &operator>>(DataStream &ds, Mesh::Vertex &v)
{
    ds >> v.position;
    ds >> v.normal;
    ds >> v.texcoord;
    return ds;
}

std::unique_ptr<Mesh> readMesh(DataStream &ds, const Material *material)
{
    std::vector<Mesh::Vertex> vertices;
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

        for (int j = 1; j < faceIndexCount - 1; ++j) {
            indices.push_back(faceIndices[0]);
            indices.push_back(faceIndices[j]);
            indices.push_back(faceIndices[j + 1]);
        }
    }

    std::unique_ptr<Mesh> mesh(new Mesh(material));
    mesh->setData(vertices, indices);
    return mesh;
}

} // namespace

Level::Level() = default;
Level::~Level() = default;

void Level::load(const char *filepath, MaterialCache *materialCache)
{
    DataStream ds(filepath);
    if (!ds) {
        panic("failed to open %s\n", filepath);
    }

    uint32_t meshCount;
    ds >> meshCount;

    std::cout << "Reading " << meshCount << " meshes\n";
    for (int i = 0; i < meshCount; ++i) {
        MaterialKey material;
        ds >> material;
        m_meshes.push_back(readMesh(ds, materialCache->cachedMaterial(material)));
    }
}

void Level::render(Renderer *renderer, const glm::mat4 &worldMatrix) const
{
    for (const auto &m : m_meshes) {
        renderer->render(m.get(), worldMatrix);
    }
}
