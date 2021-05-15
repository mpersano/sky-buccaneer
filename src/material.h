#pragma once

#include "shadermanager.h"

#include <memory>
#include <string>

namespace GL {
class Texture;
}

class DataStream;

struct MaterialKey {
    std::string name;
    ShaderManager::Program program;
    std::string baseColorTexture;
    bool operator==(const MaterialKey &other) const
    {
        return program == other.program && baseColorTexture == other.baseColorTexture;
    }
};

DataStream &operator>>(DataStream &ds, MaterialKey &material);

struct Material {
public:
    explicit Material(const MaterialKey &materialKey);
    explicit Material(ShaderManager::Program program);
    explicit Material(ShaderManager::Program program, const std::string &baseColorTexture);
    ~Material();

    ShaderManager::Program program() const { return m_program; }
    void bind() const;

private:
    ShaderManager::Program m_program;
    std::unique_ptr<GL::Texture> m_baseColor;
};
