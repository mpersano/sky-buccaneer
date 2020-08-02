#include "shadowbuffer.h"

namespace GL {

ShadowBuffer::ShadowBuffer(int width, int height)
    : m_width{ width }
    , m_height{ height }
{
    glGenTextures(1, &m_textureId);
    glGenFramebuffers(1, &m_fboId);
    // glGenRenderbuffers(1, &rbo_id_);

    // initialize texture

    // note to self: non-power of 2 textures are allowed on es >2.0 if
    // GL_MIN_FILTER is set to a function that doesn't require mipmaps
    // and texture wrap is set to GL_CLAMP_TO_EDGE

    bindTexture();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    unbindTexture();

    // initialize ShadowBuffer/renderbuffer

    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureId, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    unbind();
}

ShadowBuffer::~ShadowBuffer()
{
    glDeleteFramebuffers(1, &m_fboId);
    glDeleteTextures(1, &m_textureId);
}

void ShadowBuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);
}

void ShadowBuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowBuffer::bindTexture() const
{
    glBindTexture(GL_TEXTURE_2D, m_textureId);
}

void ShadowBuffer::unbindTexture() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace GL
