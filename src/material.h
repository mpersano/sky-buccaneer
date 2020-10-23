#pragma once

#include <memory>
#include <string>

namespace GL {
class Texture;
}

class DataStream;

struct MaterialKey {
    std::string baseColorTexture;
    bool operator==(const MaterialKey &other) const
    {
        return baseColorTexture == other.baseColorTexture;
    }
};

DataStream &operator>>(DataStream &ds, MaterialKey &material);

struct Material {
public:
    explicit Material(const MaterialKey &materialKey);
    ~Material();

    void bind() const;

private:
    std::unique_ptr<GL::Texture> m_baseColor;
};
