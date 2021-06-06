#include "entity.h"

#include "datastream.h"
#include "materialcache.h"
#include "mesh.h"
#include "renderer.h"
#include "shaderprogram.h"
#include "transformutils.h"

#include <algorithm>

#include <spdlog/spdlog.h>

#include <glm/gtx/string_cast.hpp>

namespace {

auto readMesh(DataStream &ds)
{
    std::vector<MeshVertex> vertices;
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

    auto mesh = makeMesh(GL_TRIANGLES, vertices, indices);

    std::vector<Triangle> triangles;
    for (int i = 0; i < triangleCount; ++i) {
        const auto &v0 = vertices[indices[i * 3]];
        const auto &v1 = vertices[indices[i * 3 + 1]];
        const auto &v2 = vertices[indices[i * 3 + 2]];
        triangles.push_back({ v0.position, v1.position, v2.position });
    }

    return std::tuple(triangles, std::move(mesh));
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
    uint32_t channelCount;
    ds >> channelCount;
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

bool Entity::load(const char *filepath, MaterialCache *materialCache)
{
    DataStream ds(filepath);
    if (!ds) {
        spdlog::error("Failed to open {}", filepath);
        return false;
    }

    if (!load(ds, materialCache)) {
        spdlog::error("Malformed entity file {}", filepath);
        return false;
    }

    spdlog::info("Read entity file {}, nodes:", filepath);
    for (const auto *node : m_rootNodes)
        node->dump(0);

    return true;
}

bool Entity::load(DataStream &ds, MaterialCache *materialCache)
{
    uint32_t nodeCount;
    ds >> nodeCount;

    std::generate_n(std::back_inserter(m_nodes), nodeCount, [] {
        return std::make_unique<Node>();
    });
    for (auto &node : m_nodes) {
        ds >> node->name;

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
            if (childIndex >= m_nodes.size()) {
                return false;
            }
            auto *child = m_nodes[childIndex].get();
            if (child->parent) {
                return false;
            }
            child->parent = node.get();
            node->children.push_back(child);
        }

        uint32_t actionCount;
        ds >> actionCount;
        node->actions.reserve(actionCount);
        for (int i = 0; i < actionCount; ++i) {
            node->actions.push_back(readAction(ds));
        }

        if (nodeType == NodeType::Mesh) {
            uint32_t meshCount;
            ds >> meshCount;
            node->meshes.reserve(meshCount);

            for (int i = 0; i < meshCount; ++i) {
                MaterialKey materialKey;
                ds >> materialKey;
                auto [triangles, mesh] = readMesh(ds);
                const auto *material = materialCache->cachedMaterial(materialKey);
                node->meshes.push_back({ std::move(mesh), material });
                node->collisionMesh.addTriangles(triangles);
            }
        }
    }

    for (auto &node : m_nodes) {
        if (!node->parent) {
            m_rootNodes.push_back(node.get());
        }
    }

    return true;
}

void Entity::render(Renderer *renderer, const glm::mat4 &worldMatrix, float frame) const
{
    for (const auto *node : m_rootNodes) {
        node->render(renderer, worldMatrix, frame);
    }
}

std::optional<glm::vec3> Entity::findCollision(const LineSegment &segment, const glm::mat4 &worldMatrix, float frame) const
{
    std::optional<float> collisionT;
    for (const auto *node : m_rootNodes) {
        if (const auto ot = node->intersection(segment, worldMatrix, frame)) {
            if (const auto t = *ot; !collisionT || t < collisionT) {
                collisionT = t;
            }
        }
    }
    if (!collisionT)
        return {};
    return segment.pointAt(*collisionT);
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

glm::mat4 Entity::Node::worldMatrixAt(const glm::mat4 &parentWorldMatrix, float frame) const
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
    return parentWorldMatrix * localMatrix;
}

void Entity::Node::render(Renderer *renderer, const glm::mat4 &parentWorldMatrix, float frame) const
{
    const auto worldMatrix = worldMatrixAt(parentWorldMatrix, frame);
    for (const auto &m : meshes) {
        renderer->render(m.mesh.get(), m.material, worldMatrix);
    }
    for (const auto *child : children) {
        child->render(renderer, worldMatrix, frame);
    }
}

std::optional<float> Entity::Node::intersection(const LineSegment &segment, const glm::mat4 &parentWorldMatrix, float frame) const
{
    const auto worldMatrix = worldMatrixAt(parentWorldMatrix, frame);
    const auto invWorldMatrix = glm::inverse(worldMatrix);
    const auto mapToLocal = [&invWorldMatrix](const glm::vec3 &v) {
        const auto p = invWorldMatrix * glm::vec4(v, 1.0f);
        return glm::vec3(p) / p.w;
    };
    const auto localLineSegment = LineSegment { mapToLocal(segment.from), mapToLocal(segment.to) };
    return collisionMesh.intersection(localLineSegment);
}

void Entity::Node::dump(int indent) const
{
    spdlog::info("{:>{}} {}: {} meshes, {} actions", "", indent, name, meshes.size(), actions.size());
    for (const auto *child : children)
        child->dump(indent + 1);
}
