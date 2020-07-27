#include "mesh.h"

namespace {

struct VAOBinder : NonCopyable {
    VAOBinder(GLuint vao)
    {
        glBindVertexArray(vao);
    }

    ~VAOBinder()
    {
        glBindVertexArray(0);
    }
};

} // namespace

Mesh::Mesh()
{
    glGenBuffers(1, &m_vbo);
    glGenVertexArrays(1, &m_vao);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void Mesh::setData(const std::vector<Vertex> &vertices)
{
    m_vertexCount = vertices.size();

    VAOBinder vaoBinder(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    static_assert(sizeof(glm::vec4) == 4 * sizeof(GLfloat));

    constexpr auto Stride = sizeof(Vertex);
    static_assert(Stride == 6 * sizeof(GLfloat));

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Stride, reinterpret_cast<GLvoid *>(0));

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, Stride, reinterpret_cast<GLvoid *>(sizeof(glm::vec3)));
}

void Mesh::render() const
{
    VAOBinder vaoBinder(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
}
