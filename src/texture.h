#pragma once

#include "noncopyable.h"

#include <GL/gl.h>
#include <string>

namespace GL {

class Texture : private NonCopyable
{
public:
    Texture();
    ~Texture();

    void allocate(int width, int height);
    bool load(const std::string &path);

    int width() const
    {
        return m_width;
    }

    int height() const
    {
        return m_height;
    }

    void bind() const;

private:
    void initialize(const GLvoid *data);

    int m_width = 0;
    int m_height = 0;
    GLuint m_id;
};

} // namespace GL
