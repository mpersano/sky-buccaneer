#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

#include "transform.h"

class Mesh;

class Entity
{
    friend class Renderer;
public:
    Entity();
    ~Entity();

    void load(const char* filepath);

private:
    struct Node
    {
        Transform transform;
        const Node *parent = nullptr;
        std::vector<const Node *> children;
        std::unique_ptr<Mesh> mesh; // possibly null

        void render(const glm::mat4& mvp) const;
    };

    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<const Node *> m_rootNodes;
};
