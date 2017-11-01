#pragma once

#include <glm/glm/glm.hpp>

using Index = uint32_t;

#pragma pack(push, 1)
struct Vertex
{
	//Position of a Vertex in Model Space
	float Position[3];
	//UV coordinates of the texture bound to the primitive if there is one
	float UV[2];
};
#pragma(pop)

struct UniformMatrices
{
	glm::mat4 Projection;
	glm::mat4 View;
};