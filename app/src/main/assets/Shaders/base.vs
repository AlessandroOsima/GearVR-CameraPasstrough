#version 310 es
in vec3 vPosition;
in vec2 vUV;
out vec2 outUV;
uniform mat4 UVTransform;
void main()
{
 gl_Position = vec4(vPosition.xyz,1);
 outUV = (UVTransform * vec4(vUV.xy,0,1)).xy;
}