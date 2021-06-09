#include "material.h"

#include "datastream.h"
#include "texture.h"

#include <unordered_map>

namespace {

std::string texturePath(const std::string &basename)
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

Material *cachedMaterial(const MaterialKey &key)
{
    struct KeyHasher {
        std::size_t operator()(const MaterialKey &key) const
        {
            return std::hash<std::string>()(key.baseColorTexture);
        }
    };
    static std::unordered_map<MaterialKey, std::unique_ptr<Material>, KeyHasher> cache;
    auto it = cache.find(key);
    if (it == cache.end()) {
        auto material = std::make_unique<Material>(key.program, key.baseColorTexture);
        it = cache.emplace(key, std::move(material)).first;
    }
    return it->second.get();
}
