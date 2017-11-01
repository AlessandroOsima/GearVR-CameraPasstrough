#version 310 es
precision mediump float;
in vec2 outUV;
out vec4 color;

uniform sampler2D cameraFrameSampler;
uniform mat3 Mask;
uniform float Acceleration;
void main()
{
       float Weight = 1.f/9.f;
       vec4 sum = vec4(0,0,0,1.f);
       sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,1)) * Weight;
       sum += textureOffset(cameraFrameSampler, outUV, ivec2(0,-1)) * Weight;
       sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,-1)) * Weight;

       sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,0)) * Weight;
       sum += textureOffset(cameraFrameSampler, outUV, ivec2(0,0)) * Weight;
       sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,0)) * Weight;

       sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,1)) * Weight;
       sum += textureOffset(cameraFrameSampler, outUV, ivec2(0,1)) * Weight;
       sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,1)) * Weight;

       color = vec4(sum.xyz, 1.0f);
}