#include "materialcache.h"

#include "material.h"

MaterialCache::MaterialCache() = default;
MaterialCache::~MaterialCache() = default;

std::size_t MaterialCache::KeyHasher::operator()(const MaterialKey &key) const
{
    return std::hash<std::string>()(key.baseColorTexture);
}

Material *MaterialCache::cachedMaterial(const MaterialKey &key)
{
    auto it = m_cachedMaterials.find(key);
    if (it == m_cachedMaterials.end()) {
        auto material = std::make_unique<Material>(key);
        it = m_cachedMaterials.insert(it, { key, std::move(material) });
    }
    return it->second.get();
}
