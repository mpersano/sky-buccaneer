#pragma once

#include "noncopyable.h"

#include "geometryutils.h" // BoundingBox

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>

class DataStream;

class Mesh : private NonCopyable
{
public:
    explicit Mesh(GLenum primitive = GL_TRIANGLES);
    ~Mesh();

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoord;
        bool operator==(const Vertex &other) const;
    };
    void setData(const std::vector<Vertex> &vertices, const std::vector<unsigned> &indices);

    void render() const;

    const BoundingBox &boundingBox() const { return m_boundingBox; }

private:
    void initializeBuffers(const std::vector<Vertex> &vertices, const std::vector<unsigned> &indices);
    void initializeBoundingBox(const std::vector<Vertex> &vertices);

    GLenum m_primitive;
    int m_elementCount = 0;
    GLuint m_vao;
    GLuint m_vbo[2];
    BoundingBox m_boundingBox;
};

DataStream &operator>>(DataStream &ds, Mesh::Vertex &v);
