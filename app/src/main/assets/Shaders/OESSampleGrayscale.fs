#version 310 es
#extension GL_OES_EGL_image_external_essl3 : require
precision mediump float ;
in vec2 outUV;
out vec4 color;
uniform samplerExternalOES cameraFrameSampler;

vec4 ConvertToGrayscale(in vec4 RGBIn)
{
	float color = 0.2989f * RGBIn.r + 0.5870f * RGBIn.b + 0.1140f * RGBIn.g;

	return vec4(color, color, color, 1.f);
}

void main()
{
    vec4 grayscaleOutput = ConvertToGrayscale(texture(cameraFrameSampler, outUV));
    color = grayscaleOutput;
}