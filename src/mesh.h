#pragma once

#include "noncopyable.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>

class Mesh : private NonCopyable
{
public:
    Mesh();
    ~Mesh();

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
    };
    void setData(const std::vector<Vertex> &vertices);

    void render() const;

private:
    int m_vertexCount = 0;
    GLuint m_vao;
    GLuint m_vbo;
};
