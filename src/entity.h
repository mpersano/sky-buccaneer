#pragma once

#include <memory>
#include <optional>
#include <string_view>
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
    bool setActiveAction(std::string_view node, std::string_view action);

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

    struct Action {
        std::string name;
        std::optional<RotationChannel> rotationChannel;
        std::optional<TranslationChannel> translationChannel;
        std::optional<ScaleChannel> scaleChannel;
    };

    struct Node {
        ~Node();
        void render(Renderer *renderer, const glm::mat4 &parentWorldMatrix, float frame) const;
        std::string name;
        Transform transform;
        const Node *parent = nullptr;
        std::vector<const Node *> children;
        std::unique_ptr<Mesh> mesh; // possibly null
        std::vector<std::unique_ptr<Action>> actions;
        const Action *activeAction = nullptr;
    };

    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<const Node *> m_rootNodes;
};
