#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
	GLenum primitiveType)
	: mVertices(vertices), mIndices(indices), mPrimitiveType(primitiveType)
{
	if (primitiveType == GL_PATCHES)
		glPatchParameteri(GL_PATCH_VERTICES, 4);

	vertexByteSize = (uint32_t)mVertices.size() * sizeof(Vertex);
	indexByteSize = (uint32_t)mIndices.size() * sizeof(uint32_t);

	CalculateBoundingBoxCenter();
}

void Mesh::ConfigureMesh(GLenum usage, bool isBoundingBox)
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

	// tangents
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	
	glBindVertexArray(0);

	if (isBoundingBox)
	{

	}
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

glm::vec3 Mesh::GetBoundingBoxCenter()
{
	return mBoundingBoxCenter;
}
double Mesh::GetDiagnalLength()
{
	return mDiagnalLength;
}

void Mesh::CalculateBoundingBoxCenter()
{
	float maxCoordX = std::numeric_limits<float>::min();
	float maxCoordY = std::numeric_limits<float>::min();
	float maxCoordZ = std::numeric_limits<float>::min();

	float minCoordX = std::numeric_limits<float>::max();
	float minCoordY = std::numeric_limits<float>::max();
	float minCoordZ = std::numeric_limits<float>::max();

	int num = 0;

	// 8 coordinates of bounding box vertices
	for (uint32_t i = 0; i != mVertices.size(); i++)
	{
		Vertex temp = mVertices[i];

		if (maxCoordX < temp.position.x)
			maxCoordX = temp.position.x;

		if (minCoordX > temp.position.x)
			minCoordX = temp.position.x;

		if (maxCoordY < temp.position.y)
			maxCoordY = temp.position.y;

		if (minCoordY > temp.position.y)
			minCoordY = temp.position.y;

		if (maxCoordZ < temp.position.z)
			maxCoordZ = temp.position.z;

		if (minCoordZ > temp.position.z)
			minCoordZ = temp.position.z;
	}

	std::vector<glm::vec3> tempVec;
	tempVec.push_back(glm::vec3(minCoordX, minCoordY, minCoordZ));
	tempVec.push_back(glm::vec3(minCoordX, minCoordY, maxCoordZ));
	tempVec.push_back(glm::vec3(minCoordX, maxCoordY, minCoordZ));
	tempVec.push_back(glm::vec3(minCoordX, maxCoordY, maxCoordZ));
	tempVec.push_back(glm::vec3(maxCoordX, minCoordY, minCoordZ));
	tempVec.push_back(glm::vec3(maxCoordX, minCoordY, maxCoordZ));
	tempVec.push_back(glm::vec3(maxCoordX, maxCoordY, minCoordZ));
	tempVec.push_back(glm::vec3(maxCoordX, maxCoordY, maxCoordZ));
	mBoundingBoxVertices = tempVec;

	mBoundingBoxIndices = {
			0, 2, 3, 0, 3, 1,
			7, 6, 2, 7, 2, 3,
			4, 6, 2, 4, 2, 0,
			4, 6, 7, 4, 7, 5,
			5, 7, 3, 5, 3, 1,
			5, 4, 0, 5, 0, 1
	};

	// Center of bounding box
	mBoundingBoxCenter.x = (maxCoordX + minCoordX) * 0.5f;
	mBoundingBoxCenter.y = (maxCoordY + minCoordY) * 0.5f;
	mBoundingBoxCenter.z = (maxCoordZ + minCoordZ) * 0.5f;

	// diagonal length
	mDiagnalLength = std::sqrt(std::pow(maxCoordX - mBoundingBoxCenter.x, 2) + pow(maxCoordY - mBoundingBoxCenter.y, 2) + pow(maxCoordZ - mBoundingBoxCenter.z, 2));
}