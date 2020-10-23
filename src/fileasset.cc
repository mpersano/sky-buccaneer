#include "fileasset.h"

#include "material.h"

namespace {

const std::string texturePath(const std::string &basename)
{
    return std::string("assets/textures/") + basename;
}

} // namespace

FileAsset::FileAsset(const char *path)
    : m_is(path, std::ios::binary)
{
}

void FileAsset::read(char *buf, std::streamsize size)
{
    m_is.read(buf, size);
}

template<>
std::string FileAsset::read()
{
    const int length = read<uint8_t>();
    std::string s;
    s.resize(length);
    read(s.data(), length);
    return s;
}

template<>
MaterialKey FileAsset::read()
{
    read<std::string>(); // skip name
    MaterialKey material;
    material.baseColorTexture = texturePath(read<std::string>());
    return material;
}
