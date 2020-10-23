#include "entity.h"

#include "datastream.h"
#include "materialcache.h"
#include "mesh.h"
#include "panic.h"
#include "renderer.h"
#include "shaderprogram.h"
#include "transformutils.h"

#include <algorithm>
#include <iostream>

#include <glm/gtx/string_cast.hpp>

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

    uint32_t triangleCount;
    ds >> triangleCount;

    std::vector<unsigned> indices;
    indices.reserve(3 * triangleCount);
    for (uint32_t i = 0; i < 3 * triangleCount; ++i) {
        uint32_t index;
        ds >> index;
        indices.push_back(index);
    }

    std::unique_ptr<Mesh> mesh(new Mesh(material));
    mesh->setData(vertices, indices);
    return mesh;
}

template<typename SampleT>
Action::Channel<SampleT> readChannel(DataStream &ds, uint32_t startFrame, uint32_t endFrame)
{
    Action::Channel<SampleT> channel;
    channel.startFrame = startFrame;
    std::generate_n(std::back_inserter(channel.samples), endFrame - startFrame + 1, [&ds] {
        SampleT t;
        ds >> t;
        return t;
    });
    return channel;
}

std::unique_ptr<Action> readAction(DataStream &ds)
{
    std::unique_ptr<Action> action(new Action);
    ds >> action->name;
    std::cout << "Reading action `" << action->name << "'\n";
    uint32_t channelCount;
    ds >> channelCount;
    std::cout << "Reading " << channelCount << " channels\n";
    for (int j = 0; j < channelCount; ++j) {
        enum class PathType : uint8_t { Rotation,
                                        Translation,
                                        Scale } pathType;
        ds >> reinterpret_cast<uint8_t &>(pathType);
        uint32_t startFrame;
        ds >> startFrame;
        uint32_t endFrame;
        ds >> endFrame;
        switch (pathType) {
        case PathType::Rotation:
            action->rotationChannel = readChannel<glm::quat>(ds, startFrame, endFrame);
            break;
        case PathType::Translation:
            action->translationChannel = readChannel<glm::vec3>(ds, startFrame, endFrame);
            break;
        case PathType::Scale:
            action->scaleChannel = readChannel<glm::vec3>(ds, startFrame, endFrame);
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
    DataStream ds(filepath);
    if (!ds) {
        panic("failed to open %s\n", filepath);
    }

    const auto panicUnless = [filepath](bool condition) {
        if (!condition) {
            panic("malformed entity file %s\n", filepath);
        }
    };

    uint32_t nodeCount;
    ds >> nodeCount;

    std::cout << "**** nodeCount=" << nodeCount << '\n';

    std::generate_n(std::back_inserter(m_nodes), nodeCount, [] {
        return std::make_unique<Node>();
    });
    for (auto &node : m_nodes) {
        ds >> node->name;
        std::cout << "Reading node `" << node->name << "'\n";

        enum class NodeType : uint8_t { Empty,
                                        Mesh } nodeType;
        ds >> reinterpret_cast<uint8_t &>(nodeType);

        ds >> node->transform;

        uint32_t childCount;
        ds >> childCount;
        node->children.reserve(childCount);
        for (int i = 0; i < childCount; ++i) {
            uint32_t childIndex;
            ds >> childIndex;
            panicUnless(childIndex < m_nodes.size());
            auto *child = m_nodes[childIndex].get();
            panicUnless(!child->parent);
            child->parent = node.get();
            node->children.push_back(child);
        }

        uint32_t actionCount;
        ds >> actionCount;
        std::cout << "Reading " << actionCount << " actions\n";
        node->actions.reserve(actionCount);
        for (int i = 0; i < actionCount; ++i) {
            node->actions.push_back(readAction(ds));
        }

        if (nodeType == NodeType::Mesh) {
            uint32_t meshCount;
            ds >> meshCount;
            std::cout << "Reading " << meshCount << " meshes\n";
            node->meshes.reserve(meshCount);

            for (int i = 0; i < meshCount; ++i) {
                MaterialKey material;
                ds >> material;
                node->meshes.push_back(readMesh(ds, materialCache->cachedMaterial(material)));
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
