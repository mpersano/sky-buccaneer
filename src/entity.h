#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "transform.h"

class Mesh;
class Renderer;

class Entity
{
public:
    ~Entity();

    void load(const char *filepath);
    void render(Renderer *renderer, const glm::mat4 &worldMatrix, float frame) const;

private:
    struct Node;

    struct RotationChannel {
        int startFrame;
        std::vector<glm::quat> samples; // one sample per frame
    };

    struct TranslationChannel {
        int startFrame;
        std::vector<glm::vec3> samples; // ditto
    };

    struct ScaleChannel {
        int startFrame;
        std::vector<glm::vec3> samples; // ditto
    };

    struct Node {
        ~Node();
        void render(Renderer *renderer, const glm::mat4 &parentWorldMatrix, float frame) const;
        Transform transform;
        const Node *parent = nullptr;
        std::vector<const Node *> children;
        std::unique_ptr<Mesh> mesh; // possibly null

        // XXX for now
        std::optional<RotationChannel> rotationChannel;
        std::optional<TranslationChannel> translationChannel;
        std::optional<ScaleChannel> scaleChannel;
    };

    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<const Node *> m_rootNodes;
};
