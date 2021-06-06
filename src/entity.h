#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "collisionmesh.h"
#include "transform.h"

class Mesh;
class Material;
class Renderer;
class MaterialCache;

struct Action {
    std::string name;
    template<typename SampleT>
    struct Channel {
        int startFrame;
        std::vector<SampleT> samples;
    };
    std::optional<Channel<glm::quat>> rotationChannel;
    std::optional<Channel<glm::vec3>> translationChannel;
    std::optional<Channel<glm::vec3>> scaleChannel;
};

class Entity
{
public:
    ~Entity();

    bool load(const char *filepath, MaterialCache *materialCache);
    void render(Renderer *renderer, const glm::mat4 &worldMatrix, float frame) const;
    std::optional<glm::vec3> findCollision(const LineSegment &segment, const glm::mat4 &worldMatrix, float frame) const;

    bool setActiveAction(std::string_view node, std::string_view action);

private:
    bool load(DataStream &ds, MaterialCache *materialCache);
    struct Node {
        ~Node();
        void render(Renderer *renderer, const glm::mat4 &parentWorldMatrix, float frame) const;
        std::optional<float> intersection(const LineSegment &segment, const glm::mat4 &parentWorldMatrix, float frame) const;
        glm::mat4 worldMatrixAt(const glm::mat4 &parentWorldMatrix, float frame) const;
        void dump(int indent) const;

        std::string name;
        Transform transform;
        const Node *parent = nullptr;
        std::vector<const Node *> children;
        struct MeshMaterial {
            std::unique_ptr<Mesh> mesh;
            const Material *material;
        };
        std::vector<MeshMaterial> meshes;
        CollisionMesh collisionMesh;
        std::vector<std::unique_ptr<Action>> actions;
        const Action *activeAction = nullptr;
    };

    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<const Node *> m_rootNodes;
};
