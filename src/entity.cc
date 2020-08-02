#include "entity.h"

#include "panic.h"
#include "mesh.h"
#include "shaderprogram.h"

#include <algorithm>
#include <fstream>

#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace {

template <typename T>
T read(std::istream& is)
{
    // TODO handle endianness
    T value;
    is.read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

std::unique_ptr<Mesh> readMesh(std::istream& is)
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

Entity::Entity()
    : m_shaderProgram(new GL::ShaderProgram)
{
    m_shaderProgram->addShader(GL_VERTEX_SHADER, "assets/shaders/simple.vert");
    m_shaderProgram->addShader(GL_FRAGMENT_SHADER, "assets/shaders/simple.frag");
    m_shaderProgram->link();
}

Entity::~Entity() = default;

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
    for (auto &node : m_nodes)
    {
        const auto type = read<uint8_t>(is);
        node->transform = glm::transpose(read<glm::mat4>(is));
        std::cout << "****   transform=" << glm::to_string(node->transform) << '\n';
        const auto childCount = read<uint32_t>(is);
        for (int i = 0; i < childCount; ++i)
        {
            const auto childIndex = read<uint32_t>(is);
            panicUnless(childIndex < m_nodes.size());
            auto *child = m_nodes[childIndex].get();
            panicUnless(!child->parent);
            child->parent = node.get();
            node->children.push_back(child);
        }
        if (type == 1) {
            node->mesh = readMesh(is);
        }
    }

    for (auto& node : m_nodes) {
        if (!node->parent) {
            m_rootNodes.push_back(node.get());
        }
    }
}

void Entity::render(const glm::mat4& mvp) const
{
    m_shaderProgram->bind();
    m_shaderProgram->setUniform("lightPosition", glm::vec3(0, 0, -2));
    m_shaderProgram->setUniform("color", glm::vec3(1));

    for (auto *node : m_rootNodes) {
        render(mvp, node);
    }
}

void Entity::render(const glm::mat4& mvp, const Node *node) const
{
    if (node->mesh)
    {
        std::cout << "mvp=" << glm::to_string(mvp) << '\n';
        m_shaderProgram->setUniform("mvp", mvp * node->transform);
#if 0
        m_shaderProgram->setUniform("modelMatrix", modelMatrix);
#endif
        node->mesh->render();
    }
    for (const auto *child : node->children)
    {
        render(mvp * node->transform, child);
    }
}
