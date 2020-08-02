#pragma once

#include <memory>
#include <vector>

#include "transform.h"

class Mesh;
class Renderer;

class Entity
{
public:
    ~Entity();

    void load(const char *filepath);
    void render(Renderer *renderer, const glm::mat4 &worldMatrix) const;

private:
    struct Node {
        ~Node();
        void render(Renderer *renderer, const glm::mat4 &parentWorldMatrix) const;
        Transform transform;
        const Node *parent = nullptr;
        std::vector<const Node *> children;
        std::unique_ptr<Mesh> mesh; // possibly null
    };
    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<const Node *> m_rootNodes;
};
