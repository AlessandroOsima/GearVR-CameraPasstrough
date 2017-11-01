#pragma once

#include <stdint.h>
#include <string>
#include <vector>

using TextureID = uint32_t;

struct TextureInfo
{
	int Width = 0;
	int Height = 0;
	int Format = -1;
};

class Texture
{
public:

	static const TextureID INVALID_ID = 0;


	static std::vector<TextureID> AllocateIDSet(size_t SetCount);
	Texture(unsigned int BindTarget);
	Texture(unsigned int BindTarget, TextureID WithID);

	void GenerateTextureWithSize(unsigned  int BindTarget, uint32_t Width, uint32_t Height, unsigned int Format, unsigned int InternalFormat, unsigned int Type, void * Data);

	void SetImageData(unsigned  int BindTarget, unsigned int Level, int OffsetX, int OffsetY, int Width, int Height, unsigned int Format, unsigned int Type, void * Data);

	inline TextureID GetID() const
	{
		return ID;
	}

	inline const TextureInfo & GetTextureInfo() const
	{
		return Info;
	}

	void Bind( unsigned int BindTarget, unsigned int Unit);
	void UnBind(unsigned int BindTarget);

	void InvalidateTexture();
	~Texture();

private:
	TextureID ID = Texture::INVALID_ID;
	TextureInfo Info;
};

