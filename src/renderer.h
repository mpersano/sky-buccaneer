#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <vector>

#include "camera.h"

class Mesh;
class Material;

namespace GL {
class ShadowBuffer;
class Texture;
} // namespace GL

class ShaderManager;

class Renderer
{
public:
    Renderer(ShaderManager *shaderManager, const Camera *camera);
    ~Renderer();

    void resize(int width, int height);

    void begin();
    void render(const Mesh *mesh, const Material *material, const glm::mat4 &worldMatrix);
    void end();

private:
    int m_width = 1;
    int m_height = 1;
    glm::vec3 m_lightPosition;
    std::unique_ptr<GL::ShadowBuffer> m_shadowBuffer;
    struct DrawCall {
        const Mesh *mesh;
        const Material *material;
        glm::mat4 worldMatrix;
    };
    std::vector<DrawCall> m_drawCalls;
    ShaderManager *m_shaderManager;
    const Camera *m_camera;
    Camera m_lightCamera;
};
