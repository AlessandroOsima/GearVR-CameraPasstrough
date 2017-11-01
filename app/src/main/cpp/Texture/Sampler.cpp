#include "Sampler.h"
#include <GLES3/gl3.h>
#include "GLUtilities.h"

Sampler::Sampler()
{
	glGenSamplers(1, &SamplerID);
}


void Sampler::Bind(unsigned int Unit)
{
    glCheckFunction(glSamplerParameteri(SamplerID,  GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCheckFunction(glSamplerParameteri(SamplerID,  GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glCheckFunction(glSamplerParameteri(SamplerID,  GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glCheckFunction(glSamplerParameteri(SamplerID,  GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	UnitLocation = Unit;
    glCheckFunction(glBindSampler(Unit, SamplerID));
}

void Sampler::UnBind()
{
	//Just placeholder, it actually doesn't make sense to unbind a sampler. Just change the program
}

Sampler::~Sampler()
{
    glCheckFunction(glDeleteSamplers(1, &SamplerID));
}

int Sampler::GetID() {
    return SamplerID;
}
