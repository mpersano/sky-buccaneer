#include <cassert>

#include <stb_image.h>

#include "image.h"

Image::Image() = default;

bool Image::load(const std::string &path)
{
    int channels;
    unsigned char *data = stbi_load(path.c_str(), &m_width, &m_height, &channels, 4);
    if (!data)
        return false;
    const auto *pixels = reinterpret_cast<const uint32_t *>(data);
    m_bits.assign(pixels, pixels + m_width * m_height);
    stbi_image_free(data);
    return true;
}
