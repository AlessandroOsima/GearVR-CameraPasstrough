#pragma once
#include <vector>
#include <memory>
#include "../Math/MathTypes.h"
#include <GLES3/gl3.h>

using Index = uint32_t;

class VertexData
{
public:
	VertexData(const std::vector<Vertex> & Vertices, const std::vector<Index> & Indices);
	VertexData();

	VertexData(VertexData && MeshToReplace);
	VertexData & operator=(VertexData && MeshToReplace);

	~VertexData();

	void GenerateVertexData();

	void Bind();
	void Unbind();

	inline std::vector<Vertex> & GetVertices()
	{
		return Vertices;
	}

	inline std::vector<Index> & GetIndices()
	{
		return Indices;
	}

	void UpdateVertexData();
	void UpdateIndexData();
private:
	std::vector<Vertex> Vertices;
	std::vector<Index> Indices;

	GLuint VBO;
	GLuint VEO;
	GLuint VAO;
};


namespace  MeshUtilities
{
    std::shared_ptr<VertexData> CreateFullScreenQuad(glm::vec4 Color, bool Invert);
	std::shared_ptr<VertexData> CreateRightEyeQuad();
	std::shared_ptr<VertexData> CreateLeftEyeQuad();
    std::shared_ptr<VertexData>
    CreateAspectCorrectEyeQuad(float Aspect, float Width, float Distance);
}

