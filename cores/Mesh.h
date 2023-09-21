#pragma once
#include "Stdafx.h"
#include "Utility.h"

class Mesh
{
public:
	Mesh() = default;
	Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
		GLenum primitiveType = GL_TRIANGLES);

	void ConfigureMesh(GLenum usage = GL_STATIC_DRAW);
	void DeleteMesh();

	uint32_t GetVertexAttribArray();

	uint32_t GetVertexBuffer();
	uint32_t GetIndexBuffer();
	uint32_t GetIndexCount();

	GLenum GetPrimitiveType();
	GLenum GetIndexFormat();

private:
	std::vector<Vertex> mVertices;
	UINT vertexByteSize = 0;

	std::vector<uint32_t> mIndices;
	UINT indexByteSize = 0;

	GLenum mPrimitiveType;
	GLenum mIndexFormat = GL_UNSIGNED_INT;

	uint32_t mVertexAttribArray = 0;

	uint32_t mVertexBuffer = 0;
	uint32_t mIndexBuffer = 0;
};