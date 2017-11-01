#version 310 es
#extension GL_OES_EGL_image_external_essl3 : require
precision mediump float ;
in vec2 outUV;
out vec4 color;
uniform sampler2D cameraFrameSampler;
void main()
{
    mat3 SobelY;
    SobelY[0] = vec3(-1, -2, -1);
    SobelY[1] = vec3(0, 0, 0);
    SobelY[2] = vec3(1, 2, 1);
    mat3 SobelX;
    SobelX[0] = vec3(-1, 0, 1);
    SobelX[1] = vec3(-2, 0, 2);
    SobelX[2] = vec3(-1, 0, 1);
    vec4 sum = vec4(0,0,0,1);
    //Up 1 col"
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,-1)) * SobelY[0].x;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(0,-1)) * SobelY[0].y;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,-1)) * SobelY[0].z;
    //Center\n"
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,0)) * SobelY[1].x;
    sum += vec4(texture(cameraFrameSampler, outUV).xyz, 1) * SobelY[1].y;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,0)) * SobelY[1].z;
    //Down 1 col\n"
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,1)) * SobelY[2].x;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(0,1)) * SobelY[2].y;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,1)) * SobelY[2].z;
    //Up 1 col\n"
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,-1)) * SobelX[0].x;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(0,-1)) * SobelX[0].y;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,-1)) * SobelX[0].z;
    //Center\n"
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,0)) * SobelX[1].x;
    sum += vec4(texture(cameraFrameSampler, outUV).xyz, 1) * SobelX[1].y;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,0)) * SobelX[1].z;
    //Down 1 col\n"
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(-1,1)) * SobelX[2].x;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(0,1)) * SobelX[2].y;
    sum += textureOffset(cameraFrameSampler, outUV, ivec2(1,1)) * SobelX[2].z;
    color = vec4(sum.xyz,1);
}

