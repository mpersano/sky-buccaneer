#include "texture.h"
#include "image.h"

#include <memory>

namespace {

template<typename T>
static T
next_power_of_2(T n)
{
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

} // namespace

namespace GL {

Texture::Texture()
{
    glGenTextures(1, &m_id);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

void Texture::allocate(int width, int height)
{
    m_width = width;
    m_height = height;
    initialize(nullptr);
}

bool Texture::load(const std::string &path)
{
    Image img;
    if (!img.load(path))
        return false;

    m_width = img.width();
    m_height = img.height();
    initialize(img.bits());

    return true;
}

void Texture::bind() const
{
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::initialize(const GLvoid *data)
{
    bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

} // namespace GL
