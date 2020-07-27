#include "world.h"

#include "mesh.h"
#include "panic.h"
#include "shaderprogram.h"

#include <boost/algorithm/string.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>

namespace {

std::unique_ptr<Mesh> loadMesh(const char *path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
        panic("failed to open %s\n", path);

    const auto nextInt = [&ifs] {
        // TODO handle endianness
        uint32_t value;
        ifs.read(reinterpret_cast<char *>(&value), sizeof(value));
        return value;
    };

    nextInt(); // ignore mesh count

    const auto vertexCount = nextInt();

    std::vector<Mesh::Vertex> vertices(vertexCount);
    ifs.read(reinterpret_cast<char *>(vertices.data()), vertexCount * sizeof(Mesh::Vertex));

    const auto triangleCount = nextInt();
    std::vector<unsigned> indices(3 * triangleCount);
    ifs.read(reinterpret_cast<char *>(indices.data()), 3 * triangleCount * sizeof(unsigned));

    std::unique_ptr<Mesh> mesh(new Mesh);
    mesh->setData(vertices, indices);
    return mesh;
}

} // namespace

World::World()
    : m_shaderProgram(new GL::ShaderProgram)
    , m_mesh(std::move(loadMesh("assets/meshes/scene.bin")))
{
    m_shaderProgram->addShader(GL_VERTEX_SHADER, "assets/shaders/simple.vert");
    m_shaderProgram->addShader(GL_FRAGMENT_SHADER, "assets/shaders/simple.frag");
    m_shaderProgram->link();

    glClearColor(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

World::~World() = default;

void World::resize(int width, int height)
{
    m_projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / height, 0.1f, 100.f);

    const auto eye = glm::vec3(0, 0, 3);
    const auto center = glm::vec3(0, 0, 0);
    const auto up = glm::vec3(0, 1, 0);
    m_viewMatrix = glm::lookAt(eye, center, up);

    glViewport(0, 0, width, height);
}

void World::render() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto modelMatrix = glm::rotate(glm::mat4(1), 3.0f * static_cast<float>(m_time), glm::vec3(-1, -0.5, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f));
    const auto mvp = m_projectionMatrix * m_viewMatrix * modelMatrix;

    m_shaderProgram->bind();
    m_shaderProgram->setUniform("mvp", mvp);
    m_shaderProgram->setUniform("modelMatrix", modelMatrix);
    m_shaderProgram->setUniform("lightPosition", glm::vec3(0, 0, -2));
    m_shaderProgram->setUniform("color", glm::vec3(1));
    m_mesh->render();
}

void World::update(double elapsed)
{
    m_time += elapsed;
}
