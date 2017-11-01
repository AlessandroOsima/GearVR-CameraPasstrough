#version 310 es
#extension GL_OES_EGL_image_external : require
precision mediump float ;
in vec2 outUV;
out vec4 color;
uniform samplerExternalOES cameraFrameSampler;
void main()
{
    color = vec4(texture(cameraFrameSampler, outUV).xyz,1);
}