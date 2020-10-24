#include "mesh.h"

#include <limits>
#include <tuple>

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

bool Mesh::Vertex::operator==(const Vertex &other) const
{
    return std::tie(position, normal, texcoord) == std::tie(other.position, other.normal, other.texcoord);
}

Mesh::Mesh(const Material *material, GLenum primitive)
    : m_material(material)
    , m_primitive(primitive)
{
    glGenBuffers(2, m_vbo);
    glGenVertexArrays(1, &m_vao);
}

Mesh::~Mesh()
{
    glDeleteBuffers(2, m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void Mesh::setData(const std::vector<Vertex> &vertices, const std::vector<unsigned> &indices)
{
    m_elementCount = indices.size();

    initializeBoundingBox(vertices);
    initializeBuffers(vertices, indices);
}

void Mesh::initializeBuffers(const std::vector<Vertex> &vertices, const std::vector<unsigned> &indices)
{
    VAOBinder vaoBinder(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_STATIC_DRAW);

    static_assert(sizeof(glm::vec3) == 3 * sizeof(GLfloat));
    static_assert(sizeof(glm::vec2) == 2 * sizeof(GLfloat));

    constexpr auto Stride = sizeof(Vertex);
    static_assert(Stride == 8 * sizeof(GLfloat));

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Stride, reinterpret_cast<GLvoid *>(offsetof(Vertex, position)));

    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, Stride, reinterpret_cast<GLvoid *>(offsetof(Vertex, normal)));

    // uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, Stride, reinterpret_cast<GLvoid *>(offsetof(Vertex, texcoord)));
}

void Mesh::initializeBoundingBox(const std::vector<Vertex> &vertices)
{
    for (const auto &v : vertices) {
        m_boundingBox |= v.position;
    }
}

void Mesh::render() const
{
    VAOBinder vaoBinder(m_vao);
    glDrawElements(m_primitive, m_elementCount, GL_UNSIGNED_INT, nullptr);
}
