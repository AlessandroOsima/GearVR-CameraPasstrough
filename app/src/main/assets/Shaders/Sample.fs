#version 310 es
precision mediump float ;
in vec2 outUV;
out vec4 color;
uniform sampler2D cameraFrameSampler;
void main()
{
    color = vec4(texture(cameraFrameSampler, outUV).xyz,1);
}