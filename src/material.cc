#include "material.h"

#include "texture.h"

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
