#include "entity.h"

#include "mesh.h"
#include "panic.h"
#include "renderer.h"
#include "shaderprogram.h"
#include "transformutils.h"

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

template<>
std::string read(std::istream &is)
{
    const int length = read<uint8_t>(is);
    std::string s;
    s.resize(length);
    is.read(s.data(), length);
    return s;
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

template<typename SampleT>
Action::Channel<SampleT> readChannel(std::istream &is, uint32_t startFrame, uint32_t endFrame)
{
    Action::Channel<SampleT> channel;
    channel.startFrame = startFrame;
    std::generate_n(std::back_inserter(channel.samples), endFrame - startFrame + 1, [&is] {
        return read<SampleT>(is);
    });
    return channel;
}

std::unique_ptr<Action> readAction(std::istream &is)
{
    std::unique_ptr<Action> action(new Action);
    action->name = read<std::string>(is);
    std::cout << "Reading action `" << action->name << "'\n";
    const auto channelCount = read<uint32_t>(is);
    std::cout << "Reading " << channelCount << " channels\n";
    for (int j = 0; j < channelCount; ++j) {
        enum class PathType { Rotation,
                              Translation,
                              Scale };
        const auto pathType = static_cast<PathType>(read<uint8_t>(is));
        const auto startFrame = read<uint32_t>(is);
        const auto endFrame = read<uint32_t>(is);
        switch (pathType) {
        case PathType::Rotation:
            action->rotationChannel = readChannel<glm::quat>(is, startFrame, endFrame);
            break;
        case PathType::Translation:
            action->translationChannel = readChannel<glm::vec3>(is, startFrame, endFrame);
            break;
        case PathType::Scale:
            action->scaleChannel = readChannel<glm::vec3>(is, startFrame, endFrame);
            break;
        }
    }
    return action;
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
        node->name = read<std::string>(is);
        std::cout << "Reading node `" << node->name << "'\n";

        enum class NodeType { Empty,
                              Mesh };
        const auto nodeType = static_cast<NodeType>(read<uint8_t>(is));

        node->transform = read<Transform>(is);

        const auto childCount = read<uint32_t>(is);
        node->children.reserve(childCount);
        for (int i = 0; i < childCount; ++i) {
            const auto childIndex = read<uint32_t>(is);
            panicUnless(childIndex < m_nodes.size());
            auto *child = m_nodes[childIndex].get();
            panicUnless(!child->parent);
            child->parent = node.get();
            node->children.push_back(child);
        }

        const auto actionCount = read<uint32_t>(is);
        std::cout << "Reading " << actionCount << " actions\n";
        node->actions.reserve(actionCount);
        for (int i = 0; i < actionCount; ++i) {
            node->actions.push_back(readAction(is));
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

void Entity::render(Renderer *renderer, const glm::mat4 &worldMatrix, float frame) const
{
    for (const auto *node : m_rootNodes) {
        node->render(renderer, worldMatrix, frame);
    }
}

bool Entity::setActiveAction(std::string_view nodeName, std::string_view actionName)
{
    auto nodeIt = std::find_if(m_nodes.begin(), m_nodes.end(), [&nodeName](auto &node) {
        return node->name == nodeName;
    });
    if (nodeIt == m_nodes.end()) {
        return false;
    }
    auto &node = *nodeIt;
    auto &actions = node->actions;
    auto actionIt = std::find_if(actions.begin(), actions.end(), [&actionName](auto &action) {
        return action->name == actionName;
    });
    if (actionIt == actions.end()) {
        return false;
    }
    node->activeAction = actionIt->get();
    return true;
}

template<typename ChannelT>
auto sampleAt(const ChannelT &channel, float frame)
{
    const auto startFrame = static_cast<float>(channel.startFrame);
    const auto endFrame = static_cast<float>(channel.startFrame + channel.samples.size() - 1);
    if (frame <= startFrame) {
        return channel.samples.front();
    } else if (frame < endFrame) {
        const auto sampleIndex = static_cast<int>(frame - startFrame);
        const auto s0 = channel.samples[sampleIndex];
        const auto s1 = channel.samples[sampleIndex + 1];
        return glm::mix(s0, s1, frame - std::floor(frame));
    } else {
        return channel.samples.back();
    }
}

void Entity::Node::render(Renderer *renderer, const glm::mat4 &parentWorldMatrix, float frame) const
{
    auto [translation, rotation, scale] = transform;
    if (activeAction) {
        if (activeAction->translationChannel) {
            translation = sampleAt(*activeAction->translationChannel, frame);
        }
        if (activeAction->rotationChannel) {
            rotation = sampleAt(*activeAction->rotationChannel, frame);
        }
        if (activeAction->scaleChannel) {
            scale = sampleAt(*activeAction->scaleChannel, frame);
        }
    }
    const auto localMatrix = composeTransformMatrix(translation, rotation, scale);
    const auto worldMatrix = parentWorldMatrix * localMatrix;
    if (const auto m = mesh.get()) {
        renderer->render(m, worldMatrix);
    }
    for (const auto *child : children) {
        child->render(renderer, worldMatrix, frame);
    }
}
