#pragma once

#include <memory>
#include <vector>

#include "transform.h"

class Mesh;

struct Entity {
    ~Entity();

    void load(const char *filepath);

    struct Node {
        ~Node();
        Transform transform;
        const Node *parent = nullptr;
        std::vector<const Node *> children;
        std::unique_ptr<Mesh> mesh; // possibly null
    };
    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<const Node *> m_rootNodes;
};
