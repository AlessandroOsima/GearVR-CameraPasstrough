
#include "Texture.h"
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLUtilities.h>

#include <Logger/Asserts.h>
#include <sstream>


Texture::Texture(unsigned int BindTarget)
{
    glCheckFunction(glGenTextures(1, &ID));
    //glCheckFunction(glActiveTexture(GL_TEXTURE0));
    glCheckFunction(glBindTexture(BindTarget, ID));
	glCheckFunction(glTexParameteri(BindTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	glCheckFunction(glTexParameteri(BindTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	glCheckFunction(glTexParameteri(BindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	glCheckFunction(glTexParameteri(BindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

Texture::Texture(unsigned int BindTarget, TextureID WithID)
{
    ID = WithID;
    //glCheckFunction(glActiveTexture(GL_TEXTURE0));
    glCheckFunction(glBindTexture(BindTarget, ID));
    glCheckFunction(glTexParameteri(BindTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glCheckFunction(glTexParameteri(BindTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glCheckFunction(glTexParameteri(BindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCheckFunction(glTexParameteri(BindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void Texture::GenerateTextureWithSize(unsigned  int BindTarget, uint32_t Width, uint32_t Height, unsigned int Format, unsigned int InternalFormat, unsigned int Type, void * Data)
{
	Assert(Width);
	Assert(Height);

	Info.Width = Width;
	Info.Height = Height;

    glCheckFunction(glBindTexture(BindTarget, ID));
	glCheckFunction(glTexImage2D(BindTarget, 0, InternalFormat, Width, Height, 0, Format, Type, Data));
    glCheckFunction(glTexParameteri ( BindTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST ));
    glCheckFunction(glTexParameteri ( BindTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST ));


}

void Texture::SetImageData(unsigned  int BindTarget, unsigned int Level, int OffsetX, int OffsetY, int Width, int Height, unsigned int Format, unsigned int Type, void * Data) {
	assert(Data);

	glCheckFunction(glBindTexture(BindTarget, ID));
	glCheckFunction(glTexSubImage2D(BindTarget, Level, OffsetX, OffsetY, Width, Height, Format, Type,Data));
}

void Texture::Bind( unsigned int BindTarget, unsigned int Unit)
{
    glCheckFunction(glActiveTexture(Unit));
	glCheckFunction(glBindTexture(BindTarget, ID));

}

void Texture::UnBind(unsigned int BindTarget)
{
	glCheckFunction(glBindTexture(BindTarget, 0));
}

Texture::~Texture()
{
	if (ID != Texture::INVALID_ID)
    {
        std::stringstream str;

		str.clear();

		str << "del texture " << ID << std::ends;

		Logger::GetLogger().LogString(str.str(), LogType::LOG);

        glCheckFunction(glDeleteTextures(0, &ID));
    }
}

std::vector<TextureID> Texture::AllocateIDSet(size_t SetCount)
{
    auto ids  = std::vector<TextureID>(SetCount);
    glGenTextures(SetCount, ids.data());
    return ids;
}

void Texture::InvalidateTexture()
{
    ID = Texture::INVALID_ID;
}
