#pragma once

#include <memory>
#include <unordered_map>

#include "material.h"

class MaterialCache
{
public:
    MaterialCache();
    ~MaterialCache();

    Material *cachedMaterial(const MaterialKey &key);

private:
    struct KeyHasher {
        std::size_t operator()(const MaterialKey &key) const;
    };
    std::unordered_map<MaterialKey, std::unique_ptr<Material>, KeyHasher> m_cachedMaterials;
};
