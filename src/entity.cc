#include "entity.h"

#include "mesh.h"
#include "panic.h"
#include "renderer.h"
#include "shaderprogram.h"

#include <algorithm>
#include <fstream>

#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace {

template<typename T>
T read(std::istream &is)
{
    // TODO handle endianness
    T value;
    is.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

std::unique_ptr<Mesh> readMesh(std::istream &is)
{
    const auto vertexCount = read<uint32_t>(is);

    std::vector<Mesh::Vertex> vertices(vertexCount);
    is.read(reinterpret_cast<char *>(vertices.data()), vertexCount * sizeof(Mesh::Vertex));

    const auto triangleCount = read<uint32_t>(is);
    std::vector<unsigned> indices(3 * triangleCount);
    is.read(reinterpret_cast<char *>(indices.data()), 3 * triangleCount * sizeof(unsigned));

    std::unique_ptr<Mesh> mesh(new Mesh);
    mesh->setData(vertices, indices);
    return mesh;
}

} // namespace

Entity::~Entity() = default;

Entity::Node::~Node() = default;

void Entity::load(const char *filepath)
{
    std::ifstream is(filepath, std::ios::binary);
    if (!is) {
        panic("failed to open %s\n", filepath);
    }

    const auto panicUnless = [filepath](bool condition) {
        if (!condition) {
            panic("malformed entity file %s\n", filepath);
        }
    };

    const auto nodeCount = read<uint32_t>(is);

    std::generate_n(std::back_inserter(m_nodes), nodeCount, [] {
        return std::make_unique<Node>();
    });
    for (auto &node : m_nodes) {
        enum class NodeType { Empty,
                              Mesh };
        const auto nodeType = static_cast<NodeType>(read<uint8_t>(is));
        node->transform = read<Transform>(is);
        const auto childCount = read<uint32_t>(is);
        for (int i = 0; i < childCount; ++i) {
            const auto childIndex = read<uint32_t>(is);
            panicUnless(childIndex < m_nodes.size());
            auto *child = m_nodes[childIndex].get();
            panicUnless(!child->parent);
            child->parent = node.get();
            node->children.push_back(child);
        }
        if (nodeType == NodeType::Mesh) {
            node->mesh = readMesh(is);
        }
    }

    for (auto &node : m_nodes) {
        if (!node->parent) {
            m_rootNodes.push_back(node.get());
        }
    }
}

void Entity::render(Renderer *renderer, const glm::mat4 &worldMatrix) const
{
    for (const auto *node : m_rootNodes) {
        node->render(renderer, worldMatrix);
    }
}

void Entity::Node::render(Renderer *renderer, const glm::mat4 &parentWorldMatrix) const
{
    const auto localMatrix = transform.matrix();
    const auto worldMatrix = parentWorldMatrix * localMatrix;
    if (auto m = mesh.get()) {
        renderer->render(m, worldMatrix);
    }
    for (const auto *child : children) {
        child->render(renderer, worldMatrix);
    }
}
