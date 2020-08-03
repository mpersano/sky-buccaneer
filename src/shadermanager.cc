/*
 * Copyright 2020. Hancom Inc. All rights reserved.
 *
 * http://www.hancom.com/
 */

#include "shadermanager.h"

namespace {

template<typename T, std::size_t size>
constexpr std::size_t arrayLength(T (&)[size])
{
    return size;
}

std::unique_ptr<GL::ShaderProgram>
loadProgram(ShaderManager::Program id)
{
    enum class VertexType { Solid,
                            Colored };
    struct ProgramSource {
        const char *vertexShader;
        const char *fragmentShader;
        VertexType vertexType;
    };
    static const ProgramSource programSources[] = {
        { "assets/shaders/phong.vert", "assets/shaders/phong.frag" }, // Phong
        { "assets/shaders/shadow.vert", "assets/shaders/shadow.frag" }, // Shadow
    };
    static_assert(arrayLength(programSources) == ShaderManager::NumPrograms, "expected number of programs to match");
    const auto &sources = programSources[id];
    std::unique_ptr<GL::ShaderProgram> program(new GL::ShaderProgram);
    program->addShader(GL_VERTEX_SHADER, sources.vertexShader);
    program->addShader(GL_FRAGMENT_SHADER, sources.fragmentShader);
    program->link();
    return program;
}

} // namespace

ShaderManager::~ShaderManager() = default;

void ShaderManager::useProgram(Program id)
{
    auto &cachedProgram = m_cachedPrograms[id];
    if (!cachedProgram) {
        cachedProgram.reset(new CachedProgram);
        cachedProgram->program = loadProgram(id);
        auto &uniforms = cachedProgram->uniformLocations;
        std::fill(uniforms.begin(), uniforms.end(), -1);
    }
    if (cachedProgram.get() == m_currentProgram) {
        return;
    }
    if (cachedProgram->program) {
        cachedProgram->program->bind();
    }
    m_currentProgram = cachedProgram.get();
}

int ShaderManager::uniformLocation(Uniform id)
{
    if (!m_currentProgram || !m_currentProgram->program) {
        return -1;
    }
    auto location = m_currentProgram->uniformLocations[id];
    if (location == -1) {
        static constexpr const char *uniformNames[] = {
            // clang-format off
            "mvp",
            "projectionMatrix",
            "viewMatrix",
            "modelMatrix",
            "normalMatrix",
            "lightViewProjection",
            "eyePosition",
            "lightPosition",
            "shadowMapTexture",
            // clang-format on
        };
        static_assert(arrayLength(uniformNames) == NumUniforms, "expected number of uniforms to match");

        location = m_currentProgram->program->uniformLocation(uniformNames[id]);
        m_currentProgram->uniformLocations[id] = location;
    }
    return location;
}
