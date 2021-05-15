#include "material.h"

#include "datastream.h"
#include "texture.h"

namespace {

const std::string texturePath(const std::string &basename)
{
    return std::string("assets/textures/") + basename;
}

} // namespace

DataStream &operator>>(DataStream &ds, MaterialKey &material)
{
    std::string name;
    ds >> material.name;

    std::string baseColorTexture;
    ds >> baseColorTexture;

    material.program = ShaderManager::Program::Decal;
    material.baseColorTexture = texturePath(baseColorTexture);

    return ds;
}

Material::Material(const MaterialKey &materialKey)
    : Material(materialKey.program, materialKey.baseColorTexture)
{
}

Material::Material(ShaderManager::Program program)
    : Material(program, std::string {})
{
}

Material::Material(ShaderManager::Program program, const std::string &baseColor)
    : m_program(program)
{
    if (!baseColor.empty()) {
        m_baseColor = std::make_unique<GL::Texture>();
        m_baseColor->load(baseColor);
    }
}

Material::~Material() = default;

void Material::bind() const
{
    if (m_baseColor)
        m_baseColor->bind();
}
