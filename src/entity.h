#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Mesh;

namespace GL {
class ShaderProgram;
}

class Entity
{
public:
    Entity();
    ~Entity();

    void load(const char* filepath);

    void render(const glm::mat4 &mvp) const;

private:
    struct Node
    {
        glm::mat4 transform;
        const Node *parent = nullptr;
        std::vector<const Node *> children;
        std::unique_ptr<Mesh> mesh; // can be null

        void render(const glm::mat4& mvp) const;
    };

    void render(const glm::mat4 &mvp, const Node *node) const;

    std::vector<std::unique_ptr<Node>> m_nodes;
    std::vector<const Node *> m_rootNodes;
    std::unique_ptr<GL::ShaderProgram> m_shaderProgram; // XXX no
};
