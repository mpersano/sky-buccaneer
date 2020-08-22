#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "transform.h"

class Mesh;
class Renderer;

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

    void load(const char *filepath);
    void render(Renderer *renderer, const glm::mat4 &worldMatrix, float frame) const;
    bool setActiveAction(std::string_view node, std::string_view action);

private:
    struct Node;

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
