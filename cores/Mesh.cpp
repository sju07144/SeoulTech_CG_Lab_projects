#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
	GLenum primitiveType)
	: mVertices(vertices), mIndices(indices), mPrimitiveType(primitiveType)
{
	if (primitiveType == GL_PATCHES)
		glPatchParameteri(GL_PATCH_VERTICES, 4);

	vertexByteSize = (uint32_t)mVertices.size() * sizeof(Vertex);
	indexByteSize = (uint32_t)mIndices.size() * sizeof(uint32_t);
}

void Mesh::ConfigureMesh(GLenum usage)
{
	glGenVertexArrays(1, &mVertexAttribArray);
	glGenBuffers(1, &mVertexBuffer);
	glGenBuffers(1, &mIndexBuffer);
	
	glBindVertexArray(mVertexAttribArray);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	
	glBufferData(GL_ARRAY_BUFFER, vertexByteSize, mVertices.data(), usage);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexByteSize, mIndices.data(), usage);
	
	// positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	
	// normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	
	// texCoords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	
	glBindVertexArray(0);
}
void Mesh::DeleteMesh()
{
	glDeleteVertexArrays(1, &mVertexAttribArray);
	glDeleteBuffers(1, &mIndexBuffer);
	glDeleteBuffers(1, &mVertexBuffer);
}

uint32_t Mesh::GetVertexAttribArray()
{
	return mVertexAttribArray;
}

uint32_t Mesh::GetVertexBuffer()
{
	return mVertexBuffer;
}
uint32_t Mesh::GetIndexBuffer()
{
	return mIndexBuffer;
}
uint32_t Mesh::GetIndexCount()
{
	return static_cast<uint32_t>(mIndices.size());
}

GLenum Mesh::GetPrimitiveType()
{
	return mPrimitiveType;
}
GLenum Mesh::GetIndexFormat()
{
	return mIndexFormat;
}