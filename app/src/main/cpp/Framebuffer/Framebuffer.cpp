#include "Framebuffer.h"
#include "GLUtilities.h"
#include <EGL/egl.h>

#if !defined( GL_EXT_multisampled_render_to_texture )
typedef void (GL_APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
#endif

PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;

Framebuffer::Framebuffer()
{
	glCheckFunction(glGenFramebuffers(1, &OffscreenFramebufferID));

	glFramebufferTexture2DMultisampleEXT =
			(PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress( "glFramebufferTexture2DMultisampleEXT" );
}

Framebuffer::~Framebuffer()
{
	glCheckFunction(glDeleteFramebuffers(1, &OffscreenFramebufferID));
}

void Framebuffer::BindFramebuffer(FramebufferBindType BindType)
{
	BindingTarget = BindType;

    glCheckFunction(glBindFramebuffer(static_cast<GLenum>(BindingTarget), OffscreenFramebufferID));
}

void Framebuffer::UnBindFramebuffer()
{
    glCheckFunction(glBindFramebuffer(static_cast<GLenum>(BindingTarget), 0));
}

bool Framebuffer::IsComplete()
{
	GLint boundFB = 0;

	switch (BindingTarget)
	{
	case FramebufferBindType::READ:
	{
        glCheckFunction(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &boundFB));

		break;
	}
	case FramebufferBindType::DRAW:
	{
        glCheckFunction(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &boundFB));

		break;
	}
	case FramebufferBindType::FRAMEBUFFER:
	{
        glCheckFunction(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &boundFB));

		break;
	}
	}
	bool checked = false;

	if (boundFB && boundFB == OffscreenFramebufferID)
	{
		GLenum res = glCheckFramebufferStatus(static_cast<GLenum>(BindingTarget));
        glCheckError("glCheckFramebufferStatus");
		if (res == GL_FRAMEBUFFER_COMPLETE)
		{
			checked = true;
		}
	}

	return checked;
}

void Framebuffer::BindTextureToFramebuffer(const Texture &TextureToBind, unsigned int AttachmentPoint,
										   uint32_t Level, uint32_t MultisampleCount)
{
	if(!MultisampleCount)
	{
		glCheckFunction(glFramebufferTexture2D (GL_FRAMEBUFFER, AttachmentPoint, GL_TEXTURE_2D, TextureToBind.GetID(), Level));
	} else
	{
		glCheckFunction(glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, AttachmentPoint, GL_TEXTURE_2D, TextureToBind.GetID(), Level, MultisampleCount));
	}
}

void Framebuffer::BindTextureToFramebufferWithID(TextureID Id, unsigned int AttachmentPoint,
												 uint32_t Level, uint32_t MultisampleCount)
{
	if(!MultisampleCount)
	{
		glCheckFunction(glFramebufferTexture2D (GL_FRAMEBUFFER, AttachmentPoint, GL_TEXTURE_2D, Id, Level));
	} else
	{
		glCheckFunction(glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, AttachmentPoint, GL_TEXTURE_2D, Id, Level, MultisampleCount));
	}
}

void Framebuffer::UnbindFramebufferAttachment(unsigned int AttachmentPoint)
{
    glCheckFunction(glFramebufferTexture2D (GL_FRAMEBUFFER, AttachmentPoint, GL_TEXTURE_2D, 0, 0));
}
