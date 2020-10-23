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

namespace {

DataStream &operator>>(DataStream &ds, Mesh::Vertex &v)
{
    ds >> v.position;
    ds >> v.normal;
    ds >> v.texcoord;
    return ds;
}

std::unique_ptr<Mesh> readMesh(DataStream &ds, std::vector<Face> &faces, const Material *material)
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
    }

    std::cout << "**** mesh: " << vertices.size() << ' ' << indices.size() << '\n';
    std::cout << glm::to_string(vertices[0].position) << ' ' << glm::to_string(vertices[1].position) << '\n';

    std::unique_ptr<Mesh> mesh(new Mesh(material));
    mesh->setData(vertices, indices);
    return mesh;
}

} // namespace

Level::Level()
    : m_octree(new Octree)
{
}

Level::~Level() = default;

void Level::load(const char *filepath, MaterialCache *materialCache)
{
    std::vector<Face> faces;

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
        m_meshes.push_back(readMesh(ds, faces, materialCache->cachedMaterial(material)));
    }

    m_octree->initialize(faces);
}

void Level::render(Renderer *renderer, const glm::mat4 &worldMatrix) const
{
    for (const auto &m : m_meshes) {
        renderer->render(m.get(), worldMatrix);
    }
    m_octree->render(renderer, worldMatrix);
}
