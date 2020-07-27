#include "world.h"

#include "mesh.h"
#include "panic.h"
#include "shaderprogram.h"

#include <boost/algorithm/string.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>

namespace {

std::unique_ptr<Mesh> loadMeshFromObj(const char *path)
{
    std::ifstream ifs(path);
    if (ifs.fail())
        panic("failed to open %s\n", path);

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    struct VertexIndices {
        int position_index;
        int normal_index;
    };
    using Face = std::vector<VertexIndices>;
    std::vector<Face> faces;

    std::string line;
    while (std::getline(ifs, line)) {
        std::vector<std::string> tokens;
        boost::split(tokens, line, boost::is_any_of(" \t"), boost::token_compress_on);
        if (tokens.empty())
            continue;
        if (tokens.front() == "v") {
            assert(tokens.size() == 4);
            positions.emplace_back(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
        } else if (tokens.front() == "vn") {
            assert(tokens.size() == 4);
            normals.emplace_back(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
        } else if (tokens.front() == "f") {
            Face f;
            for (auto it = std::next(tokens.begin()); it != tokens.end(); ++it) {
                std::vector<std::string> components;
                boost::split(components, *it, boost::is_any_of("/"), boost::token_compress_off);
                assert(components.size() == 3);
                f.push_back({ std::stoi(components[0]) - 1, std::stoi(components[2]) - 1 });
            }
            faces.push_back(f);
        }
    }

    std::vector<Mesh::Vertex> vertices;
    for (const auto &face : faces) {
        for (size_t i = 1; i < face.size() - 1; ++i) {
            const auto toVertex = [&positions, &normals](const auto &vertex) -> Mesh::Vertex {
                return { glm::vec4(positions[vertex.position_index], 1), glm::vec4(normals[vertex.normal_index], 1) };
            };
            const auto v0 = toVertex(face[0]);
            const auto v1 = toVertex(face[i]);
            const auto v2 = toVertex(face[i + 1]);
            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);
        }
    }

    std::unique_ptr<Mesh> mesh(new Mesh);
    mesh->setData(vertices);
    return mesh;
}

} // namespace

World::World()
    : m_shaderProgram(new GL::ShaderProgram)
    , m_mesh(std::move(loadMeshFromObj("assets/meshes/cow.obj")))
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

    auto modelMatrix = glm::rotate(glm::mat4(1), 3.0f * static_cast<float>(m_time), glm::vec3(0, 1, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
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
