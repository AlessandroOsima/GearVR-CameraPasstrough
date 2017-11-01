
#include "VertexData.h"
#include <glm/glm/glm.hpp>
#include "GLUtilities.h"


std::shared_ptr<VertexData> MeshUtilities::CreateFullScreenQuad(glm::vec4 Color, bool Invert)
{
    /*
    std::vector<Vertex> vertices = {
            { {-1,1,0}, Color, {0,0,1}, {0,1}},
            { { 1,1,0 }, Color, { 0,0,1 }, {1,1}},
            { { 1,-1,0 }, Color, { 0,0,1 }, { 1,0 }},
            { { -1,-1,0 }, Color, { 0,0,1 }, { 0,0 }},
    };
    */

    /*
    std::vector<Vertex> vertices = {
            { {-1,1,0}, {0,1}},
            { { 1,1,0 }, {1,1}},
            { { 1,-1,0 }, { 1,0 }},
            { { -1,-1,0 }, { 0,0 }},
    };
     */

    std::vector<Vertex> vertices;

	if(Invert)
	{
		vertices = {
			{ {-1,1,0}, {0,0}},
			{ { 1,1,0 }, {1,0}},
			{ { 1,-1,0 }, { 1,1 }},
			{ { -1,-1,0 }, { 0,1 }},
		};
	}
	else
	{
		vertices = {
				{ {-1.f,1,0}, {0,1}},
				{ { 1.f,1,0 }, {1,1}},
				{ { 1.f,-1,0 }, { 1,0 }},
				{ { -1.f,-1,0 }, { 0,0 }},
		};
	}

    std::vector<Index> indices = {0,1,2,0,2,3};

    std::shared_ptr<VertexData> vData = std::make_shared<VertexData>(vertices, indices);



    return vData;
}

std::shared_ptr<VertexData> MeshUtilities::CreateLeftEyeQuad()
{

    std::vector<Vertex> vertices;

    vertices = {
            { {-2.f,   2,   -5.f}, {0,1}},
            { { 2.f,   2,   -5.f}, {1,1}},
            { { 2.f,  -2,   -5.f}, { 1,0 }},
            { { -2.f, -2,   -5.f}, { 0,0 }},
    };


    std::vector<Index> indices = {0,1,2,0,2,3};

    std::shared_ptr<VertexData> vData = std::make_shared<VertexData>(vertices, indices);



    return vData;
}

std::shared_ptr<VertexData> MeshUtilities::CreateRightEyeQuad()
{
    std::vector<Vertex> vertices;

    vertices = {
            { {-2.f,   2,   -5.f}, {0,1}},
            { { 2.f,   2,   -5.f}, {1,1}},
            { { 2.f,  -2,   -5.f}, { 1,0 }},
            { { -2.f, -2,   -5.f}, { 0,0 }},
    };


    std::vector<Index> indices = {0,1,2,0,2,3};

    std::shared_ptr<VertexData> vData = std::make_shared<VertexData>(vertices, indices);



    return vData;
}

std::shared_ptr<VertexData>
MeshUtilities::CreateAspectCorrectEyeQuad(float Aspect, float Width, float Distance)
{
    float CorrectHeight = Width * Aspect;
    std::vector<Vertex> vertices;

    vertices = {
            { {-Width,   CorrectHeight,   Distance}, {0,1}},
            { { Width,   CorrectHeight,   Distance}, {1,1}},
            { { Width,  -CorrectHeight,   Distance}, { 1,0 }},
            { { -Width, -CorrectHeight,   Distance}, { 0,0 }},
    };


    std::vector<Index> indices = {0,1,2,0,2,3};

    std::shared_ptr<VertexData> vData = std::make_shared<VertexData>(vertices, indices);



    return vData;
}

VertexData::VertexData(const std::vector<Vertex> & Vertices, const std::vector<Index> & Indices) : Vertices(Vertices), Indices(Indices)
{
	GenerateVertexData();
}

VertexData::VertexData() : VEO(0), VBO(0), VAO(0)
{

}

VertexData::VertexData(VertexData && MeshToReplace)
{
	Vertices = std::move(MeshToReplace.Vertices);
	Indices = std::move(MeshToReplace.Indices);

	VBO = MeshToReplace.VBO;
	MeshToReplace.VBO = 0;

	VEO = MeshToReplace.VEO;
	MeshToReplace.VEO = 0;

	VAO = MeshToReplace.VAO;
	MeshToReplace.VAO = 0;

}

VertexData & VertexData::operator=(VertexData && MeshToReplace)
{
	Vertices = std::move(MeshToReplace.Vertices);
	Indices = std::move(MeshToReplace.Indices);

	VBO = MeshToReplace.VBO;
	MeshToReplace.VBO = 0;

	VEO = MeshToReplace.VEO;
	MeshToReplace.VEO = 0;

	VAO = MeshToReplace.VAO;
	MeshToReplace.VAO = 0;

	return *this;
}

VertexData::~VertexData()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &VEO);

	glDeleteVertexArrays(1, &VAO);
}

void VertexData::GenerateVertexData()
{

	glCheckFunction(glGenBuffers(1, &VEO));


	glCheckFunction(glGenBuffers(1, &VBO));

	glCheckFunction(glGenVertexArrays(1, &VAO));
	glCheckFunction(glBindVertexArray(VAO));

	glCheckFunction(glBindBuffer(GL_ARRAY_BUFFER, VBO));
	glCheckFunction(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VEO));

	glCheckFunction(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * Indices.size(), Indices.data(), GL_STATIC_DRAW));
	glCheckFunction(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), Vertices.data(), GL_STATIC_DRAW));


	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Color
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 3));
	//glEnableVertexAttribArray(1);

	//Normal
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)((sizeof(float) * 3 + sizeof(float) * 4)));
	//glEnableVertexAttribArray(2);

	//UV
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)((sizeof(float) * 3 + sizeof(float) * 4 + sizeof(float) * 3)));
	//glEnableVertexAttribArray(3);
    //Only with UV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)((sizeof(float) * 3)));
    glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void VertexData::Bind()
{
	glCheckFunction(glBindVertexArray(VAO));
}

void VertexData::Unbind()
{
	glCheckFunction(glBindVertexArray(0));
}

void VertexData::UpdateVertexData()
{
    GLint  CurrentVBO = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &CurrentVBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), Vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, CurrentVBO);
}

void VertexData::UpdateIndexData()
{
    GLint  CurrentVEO = 0;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &CurrentVEO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VEO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * Indices.size(), Indices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CurrentVEO);
}
