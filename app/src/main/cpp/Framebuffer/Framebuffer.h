#pragma once
#include "Texture/Texture.h"
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>


enum class FramebufferBindType : unsigned int
{
	READ = GL_READ_FRAMEBUFFER,
	DRAW = GL_DRAW_FRAMEBUFFER,
	FRAMEBUFFER = GL_FRAMEBUFFER,
};

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();

	void BindTextureToFramebuffer(const Texture &TextureToBind, unsigned int AttachmentPoint,
                                      uint32_t Level, uint32_t MultisampleCount = 0);
	void BindTextureToFramebufferWithID(TextureID Id, unsigned int AttachmentPoint,
                                            uint32_t Level, uint32_t MultisampleCount = 0);
	void UnbindFramebufferAttachment(unsigned int AttachmentPoint);
	void BindFramebuffer(FramebufferBindType BindType);
	void UnBindFramebuffer();
	bool IsComplete();

	inline uint32_t GetID() const
	{
		return OffscreenFramebufferID;
	}

	FramebufferBindType BindingTarget;


private:
	GLuint OffscreenFramebufferID;
};

