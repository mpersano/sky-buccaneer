#include "octree.h"

#include "mesh.h"
#include "renderer.h"

#include <glm/gtx/string_cast.hpp>

#include <algorithm>
#include <iostream>
#include <set>

#define DRAW_NODE_BOXES 1
#define DRAW_POLYGON_EDGES 1

namespace OctreePrivate {

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

struct Plane {
    glm::vec3 point;
    glm::vec3 normal;
};

struct Node {
    virtual ~Node() = default;
    BoundingBox boundingBox;
#if DRAW_NODE_BOXES
    std::unique_ptr<Mesh> boxMesh;
#endif
    virtual void render(Renderer *renderer, const glm::mat4 &worldMatrix) const = 0;
};

struct LeafNode : Node {
    void render(Renderer *renderer, const glm::mat4 &worldMatrix) const override;
    std::vector<std::unique_ptr<Mesh>> meshes;
};

struct InternalNode : Node {
    void render(Renderer *renderer, const glm::mat4 &worldMatrix) const override;
    std::array<std::unique_ptr<Node>, 8> children;
};

auto split(const Face &face, const Plane &plane)
{
    Face frontFace, backFace;
    frontFace.material = backFace.material = face.material;

    const auto &verts = face.vertices;
    for (int i = 0; i < verts.size(); ++i) {
        const auto isBehind = [&plane](const glm::vec3 &v) {
            return glm::dot(v - plane.point, plane.normal) < 0;
        };

        const auto &v0 = verts[i];
        const auto &v1 = verts[(i + 1) % verts.size()];

        const auto b0 = isBehind(v0.position);
        const auto b1 = isBehind(v1.position);

        if (b0) {
            frontFace.vertices.push_back(v0);
        } else {
            backFace.vertices.push_back(v0);
        }

        if (b0 != b1) {
            const auto t = glm::dot(plane.point - v0.position, plane.normal) / glm::dot(v1.position - v0.position, plane.normal);
            const auto m = Face::Vertex { glm::mix(v0.position, v1.position, t), glm::normalize(glm::mix(v0.normal, v1.normal, t)), glm::mix(v0.texcoord, v1.texcoord, t) };
            frontFace.vertices.push_back(m);
            backFace.vertices.push_back(m);
        }
    }

    return std::pair(frontFace, backFace);
}

std::unique_ptr<Node> initializeNode(const BoundingBox &box, const std::vector<Face> &faces);

struct VertexHasher {
    std::size_t operator()(const Mesh::Vertex &vertex) const
    {
        std::hash<float> hasher;

        size_t seed = 0;
        const auto hashCombine = [&seed, &hasher](float value) {
            auto hash = hasher(value);
            hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash;
        };

        hashCombine(vertex.position.x);
        hashCombine(vertex.position.y);
        hashCombine(vertex.position.z);

        hashCombine(vertex.normal.x);
        hashCombine(vertex.normal.y);
        hashCombine(vertex.normal.z);

        hashCombine(vertex.texcoord.x);
        hashCombine(vertex.texcoord.y);

        return seed;
    }
};

std::unique_ptr<Node> initializeLeafNode(const BoundingBox &box, const std::vector<Face> &faces)
{
    auto node = std::make_unique<LeafNode>();
    node->boundingBox = box;

    std::set<const Material *> materials;
    std::transform(faces.begin(), faces.end(), std::inserter(materials, materials.begin()),
                   [](const auto &face) { return face.material; });

    for (const auto *material : materials) {
        const auto toMeshVertex = [](const Face::Vertex &faceVertex) {
            return Mesh::Vertex { faceVertex.position, faceVertex.normal, faceVertex.texcoord };
        };

        std::vector<Mesh::Vertex> vertices;
        std::unordered_map<Mesh::Vertex, int, VertexHasher> vertexIndex;
        for (auto &face : faces) {
            if (face.material != material) {
                continue;
            }
            for (auto &faceVertex : face.vertices) {
                const auto v = toMeshVertex(faceVertex);
                auto it = vertexIndex.find(v);
                if (it == vertexIndex.end()) {
                    vertices.push_back(v);
                    vertexIndex.insert(it, { v, vertexIndex.size() });
                }
            }
        }

        std::vector<unsigned> indices;
#if DRAW_POLYGON_EDGES
        std::vector<unsigned> edgeIndices;
#endif
        for (auto &face : faces) {
            const auto &vertices = face.vertices;
            std::vector<unsigned> faceIndices;
            faceIndices.reserve(vertices.size());
            std::transform(vertices.begin(), vertices.end(), std::back_inserter(faceIndices),
                           [toMeshVertex, &vertexIndex](const Face::Vertex &faceVertex) {
                               return vertexIndex[toMeshVertex(faceVertex)];
                           });
            for (int i = 1; i < faceIndices.size() - 1; ++i) {
                indices.push_back(faceIndices[0]);
                indices.push_back(faceIndices[i]);
                indices.push_back(faceIndices[i + 1]);
            }
#if DRAW_POLYGON_EDGES
            for (int i = 0; i < faceIndices.size(); ++i) {
                edgeIndices.push_back(faceIndices[i]);
                edgeIndices.push_back(faceIndices[(i + 1) % faceIndices.size()]);
            }
#endif
        }

        auto mesh = std::make_unique<Mesh>(material);
        mesh->setData(vertices, indices);
        node->meshes.push_back(std::move(mesh));

#if DRAW_POLYGON_EDGES
        auto edgeMesh = std::make_unique<Mesh>(nullptr, GL_LINES);
        edgeMesh->setData(vertices, edgeIndices);
        node->meshes.push_back(std::move(edgeMesh));
#endif
    }

    return node;
}

std::unique_ptr<Node> initializeInternalNode(const BoundingBox &box, const std::vector<Face> &faces)
{
    auto node = std::make_unique<InternalNode>();
    node->boundingBox = box;

    for (auto &child : node->children) {
        assert(!child);
    }

    const auto center = 0.5f * (box.min + box.max);

    std::array<std::vector<Face>, 8> childFaces;
    for (const auto &f : faces) {
        std::array<Face, 2> yzFaces;
        const auto yz = Plane { center, { 1, 0, 0 } };
        std::tie(yzFaces[0], yzFaces[1]) = split(f, yz);

        const auto xz = Plane { center, { 0, 1, 0 } };
        std::array<Face, 4> xzFaces;
        std::tie(xzFaces[0], xzFaces[2]) = split(yzFaces[0], xz);
        std::tie(xzFaces[1], xzFaces[3]) = split(yzFaces[1], xz);

        const auto xy = Plane { center, { 0, 0, 1 } };
        std::array<Face, 8> xyFaces;
        std::tie(xyFaces[0], xyFaces[4]) = split(xzFaces[0], xy);
        std::tie(xyFaces[1], xyFaces[5]) = split(xzFaces[1], xy);
        std::tie(xyFaces[2], xyFaces[6]) = split(xzFaces[2], xy);
        std::tie(xyFaces[3], xyFaces[7]) = split(xzFaces[3], xy);

        for (int i = 0; i < 8; ++i) {
            if (!xyFaces[i].vertices.empty()) {
                childFaces[i].push_back(xyFaces[i]);
            }
        }
    }

    for (int i = 0; i < 8; ++i) {
        if (childFaces[i].empty()) {
            continue;
        }

        BoundingBox childBox;

        if ((i & 1) == 0) {
            childBox.min.x = box.min.x;
            childBox.max.x = 0.5f * (box.min.x + box.max.x);
        } else {
            childBox.min.x = 0.5f * (box.min.x + box.max.x);
            childBox.max.x = box.max.x;
        }

        if ((i & 2) == 0) {
            childBox.min.y = box.min.y;
            childBox.max.y = 0.5f * (box.min.y + box.max.y);
        } else {
            childBox.min.y = 0.5f * (box.min.y + box.max.y);
            childBox.max.y = box.max.y;
        }

        if ((i & 4) == 0) {
            childBox.min.z = box.min.z;
            childBox.max.z = 0.5f * (box.min.z + box.max.z);
        } else {
            childBox.min.z = 0.5f * (box.min.z + box.max.z);
            childBox.max.z = box.max.z;
        }

        auto childNode = initializeNode(childBox, childFaces[i]);
        assert(childNode);
        node->children[i] = std::move(childNode);
    }

    return node;
}

std::unique_ptr<Node> initializeNode(const BoundingBox &box, const std::vector<Face> &faces)
{
    constexpr auto MaxFacesPerLeafNode = 200;

    std::unique_ptr<Node> node;
    if (faces.size() <= MaxFacesPerLeafNode) {
        node = initializeLeafNode(box, faces);
    } else {
        node = initializeInternalNode(box, faces);
    }
    assert(node);

#if DRAW_NODE_BOXES
    std::vector<Mesh::Vertex> boxVerts(8);
    for (int i = 0; i < 8; ++i) {
        float x = ((i & 1) == 0) ? box.min.x : box.max.x;
        float y = ((i & 2) == 0) ? box.min.y : box.max.y;
        float z = ((i & 4) == 0) ? box.min.z : box.max.z;
        boxVerts[i] = Mesh::Vertex { glm::vec3(x, y, z), {}, {} };
    }
    std::vector<unsigned> boxIndices = {
        0, 1,
        1, 3,
        3, 2,
        2, 0,
        4, 5,
        5, 7,
        7, 6,
        6, 4,
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };
    node->boxMesh = std::make_unique<Mesh>(nullptr, GL_LINES);
    node->boxMesh->setData(boxVerts, boxIndices);
#endif

    return node;
}

void LeafNode::render(Renderer *renderer, const glm::mat4 &worldMatrix) const
{
#if DRAW_NODE_BOXES
    renderer->render(boxMesh.get(), worldMatrix);
#endif
    for (auto &mesh : meshes) {
        renderer->render(mesh.get(), worldMatrix);
    }
}

void InternalNode::render(Renderer *renderer, const glm::mat4 &worldMatrix) const
{
#if DRAW_NODE_BOXES
    renderer->render(boxMesh.get(), worldMatrix);
#endif
    for (auto &child : children) {
        if (child) {
            child->render(renderer, worldMatrix);
        }
    }
}

} // namespace OctreePrivate

Octree::Octree() = default;
Octree::~Octree() = default;

void Octree::initialize(const std::vector<Face> &faces)
{
    OctreePrivate::BoundingBox box;
    box.min = glm::vec3(std::numeric_limits<float>::max());
    box.max = glm::vec3(std::numeric_limits<float>::min());
    for (const auto &f : faces) {
        for (auto &v : f.vertices) {
            box.min = glm::min(v.position, box.min);
            box.max = glm::max(v.position, box.max);
        }
    }
    m_root = initializeNode(box, faces);
}

void Octree::render(Renderer *renderer, const glm::mat4 &worldMatrix) const
{
    if (m_root) {
        m_root->render(renderer, worldMatrix);
    }
}
