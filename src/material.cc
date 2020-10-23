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
    ds >> name;

    std::string baseColorTexture;
    ds >> baseColorTexture;

    material.baseColorTexture = texturePath(baseColorTexture);

    return ds;
}

Material::Material(const MaterialKey &materialKey)
    : m_baseColor(new GL::Texture)
{
    m_baseColor->load(materialKey.baseColorTexture);
}

Material::~Material() = default;

void Material::bind() const
{
    m_baseColor->bind();
}
