#include "entity.h"

#include "fileasset.h"
#include "materialcache.h"
#include "mesh.h"
#include "panic.h"
#include "renderer.h"
#include "shaderprogram.h"
#include "transformutils.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <glm/gtx/string_cast.hpp>

namespace {

std::unique_ptr<Mesh> readMesh(FileAsset &f, const Material *material)
{
    const auto vertexCount = f.read<uint32_t>();

    std::vector<Mesh::Vertex> vertices(vertexCount);
    f.read(reinterpret_cast<char *>(vertices.data()), vertexCount * sizeof(Mesh::Vertex));

    const auto triangleCount = f.read<uint32_t>();
    std::vector<unsigned> indices(3 * triangleCount);
    f.read(reinterpret_cast<char *>(indices.data()), 3 * triangleCount * sizeof(unsigned));

    std::unique_ptr<Mesh> mesh(new Mesh(material));
    mesh->setData(vertices, indices);
    return mesh;
}

template<typename SampleT>
Action::Channel<SampleT> readChannel(FileAsset &f, uint32_t startFrame, uint32_t endFrame)
{
    Action::Channel<SampleT> channel;
    channel.startFrame = startFrame;
    std::generate_n(std::back_inserter(channel.samples), endFrame - startFrame + 1, [&f] {
        return f.read<SampleT>();
    });
    return channel;
}

std::unique_ptr<Action> readAction(FileAsset &f)
{
    std::unique_ptr<Action> action(new Action);
    action->name = f.read<std::string>();
    std::cout << "Reading action `" << action->name << "'\n";
    const auto channelCount = f.read<uint32_t>();
    std::cout << "Reading " << channelCount << " channels\n";
    for (int j = 0; j < channelCount; ++j) {
        enum class PathType { Rotation,
                              Translation,
                              Scale };
        const auto pathType = static_cast<PathType>(f.read<uint8_t>());
        const auto startFrame = f.read<uint32_t>();
        const auto endFrame = f.read<uint32_t>();
        switch (pathType) {
        case PathType::Rotation:
            action->rotationChannel = readChannel<glm::quat>(f, startFrame, endFrame);
            break;
        case PathType::Translation:
            action->translationChannel = readChannel<glm::vec3>(f, startFrame, endFrame);
            break;
        case PathType::Scale:
            action->scaleChannel = readChannel<glm::vec3>(f, startFrame, endFrame);
            break;
        }
    }
    return action;
}

} // namespace

Entity::~Entity() = default;

Entity::Node::~Node() = default;

void Entity::load(const char *filepath, MaterialCache *materialCache)
{
    FileAsset f(filepath);
    if (!f) {
        panic("failed to open %s\n", filepath);
    }

    const auto panicUnless = [filepath](bool condition) {
        if (!condition) {
            panic("malformed entity file %s\n", filepath);
        }
    };

    const auto nodeCount = f.read<uint32_t>();

    std::generate_n(std::back_inserter(m_nodes), nodeCount, [] {
        return std::make_unique<Node>();
    });
    for (auto &node : m_nodes) {
        node->name = f.read<std::string>();
        std::cout << "Reading node `" << node->name << "'\n";

        enum class NodeType { Empty,
                              Mesh };
        const auto nodeType = static_cast<NodeType>(f.read<uint8_t>());

        node->transform = f.read<Transform>();

        const auto childCount = f.read<uint32_t>();
        node->children.reserve(childCount);
        for (int i = 0; i < childCount; ++i) {
            const auto childIndex = f.read<uint32_t>();
            panicUnless(childIndex < m_nodes.size());
            auto *child = m_nodes[childIndex].get();
            panicUnless(!child->parent);
            child->parent = node.get();
            node->children.push_back(child);
        }

        const auto actionCount = f.read<uint32_t>();
        std::cout << "Reading " << actionCount << " actions\n";
        node->actions.reserve(actionCount);
        for (int i = 0; i < actionCount; ++i) {
            node->actions.push_back(readAction(f));
        }

        if (nodeType == NodeType::Mesh) {
            const auto meshCount = f.read<uint32_t>();
            std::cout << "Reading " << meshCount << " meshes\n";
            node->meshes.reserve(meshCount);

            for (int i = 0; i < meshCount; ++i) {
                const auto material = f.read<MaterialKey>();
                node->meshes.push_back(readMesh(f, materialCache->cachedMaterial(material)));
            }
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
    for (const auto &m : meshes) {
        renderer->render(m.get(), worldMatrix);
    }
    for (const auto *child : children) {
        child->render(renderer, worldMatrix, frame);
    }
}
