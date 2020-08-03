#pragma once

#include "shaderprogram.h"

#include <array>
#include <memory>

class ShaderManager
{
public:
    ~ShaderManager();

    enum Program {
        Phong,
        Shadow,
        NumPrograms
    };
    void useProgram(Program program);

    enum Uniform {
        MVP,
        ProjectionMatrix,
        ViewMatrix,
        ModelMatrix,
        NormalMatrix,
        LightViewProjection,
        EyePosition,
        LightPosition,
        ShadowMapTexture,
        NumUniforms
    };

    template<typename T>
    void setUniform(Uniform uniform, T &&value)
    {
        if (!m_currentProgram)
            return;
        m_currentProgram->program->setUniform(uniformLocation(uniform), std::forward<T>(value));
    }

private:
    int uniformLocation(Uniform uniform);

    struct CachedProgram {
        std::unique_ptr<GL::ShaderProgram> program;
        std::array<int, Uniform::NumUniforms> uniformLocations;
    };
    std::array<std::unique_ptr<CachedProgram>, Program::NumPrograms> m_cachedPrograms;
    CachedProgram *m_currentProgram = nullptr;
};
