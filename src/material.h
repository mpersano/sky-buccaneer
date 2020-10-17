#pragma once

#include <memory>
#include <string>

namespace GL {
class Texture;
}

struct MaterialKey {
    std::string baseColorTexture;
    bool operator==(const MaterialKey &other) const
    {
        return baseColorTexture == other.baseColorTexture;
    }
};

struct Material {
public:
    Material(const MaterialKey &materialKey);
    ~Material();

    void bind() const;

private:
    std::unique_ptr<GL::Texture> m_baseColor;
};
