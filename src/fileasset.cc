#include "fileasset.h"

FileAsset::FileAsset(const std::string &path)
    : m_is(path.c_str(), std::ios::binary)
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
