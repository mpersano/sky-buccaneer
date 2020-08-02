#pragma once

#include "noncopyable.h"

#include <GL/glew.h>

namespace GL {

class ShadowBuffer : private NonCopyable
{
public:
    ShadowBuffer(int width, int height);
    ~ShadowBuffer();

    void bind() const;
    void unbind() const;

    void bindTexture() const;
    void unbindTexture() const;

    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    int m_width;
    int m_height;
    GLuint m_textureId;
    GLuint m_fboId;
};

} // namespace GL
