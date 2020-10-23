#pragma once

#include <fstream>

class FileAsset
{
public:
    explicit FileAsset(const std::string &path);

    bool operator!() const { return !m_is; }

    void read(char *buf, std::streamsize size);

template<typename T>
T read()
{
    // TODO handle endianness
    T value;
    read(reinterpret_cast<char *>(&value), sizeof(value));
    return value;
}

private:
    std::ifstream m_is;
};
