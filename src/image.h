#pragma once

#include "noncopyable.h"

#include <string>
#include <vector>

class Image : private NonCopyable
{
public:
    Image();

    bool load(const std::string &path);

    int width() const
    {
        return m_width;
    }

    int height() const
    {
        return m_height;
    }

    const uint32_t *bits() const
    {
        return m_bits.data();
    }

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<uint32_t> m_bits;
};
