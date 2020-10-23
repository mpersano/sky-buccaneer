#pragma once

#include <fstream>

class FileAsset
{
public:
    explicit FileAsset(const char *path);

    void read(char *buf, std::streamsize size);
    template<typename T>
    T read()
    {
        // TODO handle endianness
        T value;
        read(reinterpret_cast<char *>(&value), sizeof(value));
        return value;
    }

    bool operator!() const { return !m_is; }

private:
    std::ifstream m_is;
};
