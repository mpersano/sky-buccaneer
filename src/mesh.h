#pragma once

#include "noncopyable.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

class Material;

class Mesh : private NonCopyable
{
public:
    explicit Mesh(const Material *material);
    ~Mesh();

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
    };
    void setData(const std::vector<Vertex> &vertices, const std::vector<unsigned> &indices);

    void render() const;

    const BoundingBox &boundingBox() const { return m_boundingBox; }

    const Material *material() const { return m_material; }

private:
    void initializeBuffers(const std::vector<Vertex> &vertices, const std::vector<unsigned> &indices);
    void initializeBoundingBox(const std::vector<Vertex> &vertices);

    const Material *m_material;
    int m_elementCount = 0;
    GLuint m_vao;
    GLuint m_vbo[2];
    BoundingBox m_boundingBox;
};
