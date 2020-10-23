#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Mesh;
class Renderer;
class Material;

struct Face {
    const Material *material;
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
    };
    std::vector<Vertex> vertices;
};

namespace OctreePrivate {
class Node;
}

struct Octree {
public:
    Octree();
    ~Octree();

    void initialize(const std::vector<Face> &faces);

    void render(Renderer *renderer, const glm::mat4 &worldMatrix) const;

private:
    std::unique_ptr<OctreePrivate::Node> m_root;
};
