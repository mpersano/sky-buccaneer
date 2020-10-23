#include "level.h"

#include "fileasset.h"
#include "material.h"
#include "materialcache.h"
#include "mesh.h"
#include "panic.h"
#include "renderer.h"

#include <iostream>

namespace {

std::unique_ptr<Mesh> readMesh(FileAsset &f, const Material *material)
{
    const auto vertexCount = f.read<uint32_t>();

    std::vector<Mesh::Vertex> vertices(vertexCount);
    f.read(reinterpret_cast<char *>(vertices.data()), vertexCount * sizeof(Mesh::Vertex));

    const auto faceCount = f.read<uint32_t>();

    std::vector<unsigned> indices;

    for (int i = 0; i < faceCount; ++i) {
        const auto faceIndexCount = f.read<uint8_t>();
        std::vector<uint32_t> faceIndices(faceIndexCount);
        f.read(reinterpret_cast<char *>(faceIndices.data()), faceIndexCount * sizeof(uint32_t));
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
    FileAsset f(filepath);
    if (!f) {
        panic("failed to open %s\n", filepath);
    }

    const auto meshCount = f.read<uint32_t>();
    std::cout << "Reading " << meshCount << " meshes\n";
    for (int i = 0; i < meshCount; ++i) {
        const auto material = f.read<MaterialKey>();
        m_meshes.push_back(readMesh(f, materialCache->cachedMaterial(material)));
    }
}

void Level::render(Renderer *renderer, const glm::mat4 &worldMatrix) const
{
    for (const auto &m : m_meshes) {
        renderer->render(m.get(), worldMatrix);
    }
}
