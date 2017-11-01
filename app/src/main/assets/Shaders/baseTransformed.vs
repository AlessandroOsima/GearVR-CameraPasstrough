#version 310 es
in vec3 vPosition;
in vec2 vUV;
out vec2 outUV;
uniform mat4 UVTransform;

uniform mat4 ViewTransform;
uniform mat4 ProjectionTransform;
uniform mat4 ModelTransform;

void main()
{
 gl_Position = ProjectionTransform * (ViewTransform * (ModelTransform * vec4(vPosition.xyz,1)));
 outUV = (UVTransform * vec4(vUV.xy,0,1)).xy;
}