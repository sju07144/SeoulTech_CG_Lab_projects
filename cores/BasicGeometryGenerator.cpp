#include "BasicGeometryGenerator.h"
#include "Mesh.h"

Mesh BasicGeometryGenerator::CreateBox(float width, float height, float depth)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	//
	// Create the vertices.
	//

	Vertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
	v[1] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[2] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[3] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);

	// Fill in the back face vertex data.
	v[4] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f);
	v[5] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[6] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[7] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f);

	// Fill in the left face vertex data.
	v[8] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[9] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[10] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[11] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

	// Fill in the right face vertex data.
	v[12] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f);
	v[13] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
	v[14] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f);
	v[15] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, -1.0f);

	// Fill in the top face vertex data.
	v[16] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f);
	v[17] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[18] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[19] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);

	// Fill in the bottom face vertex data.
	v[20] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f);
	v[21] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[22] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[23] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f);

	vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	uint32_t i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the right face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the left face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the top face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the bottom face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	indices.assign(&i[0], &i[36]);

	return Mesh(vertices, indices);
}

Mesh BasicGeometryGenerator::CreateGrid(float width, float depth, uint32_t m, uint32_t n)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	uint32_t vertexCount = m * n;
	uint32_t faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	vertices.resize(vertexCount);
	for (uint32_t i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (uint32_t j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			vertices[i * n + j].position = glm::vec3(x, 0.0f, z);
			vertices[i * n + j].normal = glm::vec3(0.0f, 1.0f, 0.0f);

			// Stretch texture over grid.
			vertices[i * n + j].texCoord.x = j * du;
			vertices[i * n + j].texCoord.y = i * dv;

			vertices[i * n + j].tangent = glm::vec3(1.0f, 0.0f, 0.0f);
		}
	}

	//
	// Create the indices.
	//

	indices.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32_t k = 0;
	for (uint32_t i = 0; i < m - 1; ++i)
	{
		for (uint32_t j = 0; j < n - 1; ++j)
		{
			indices[k] = i * n + j;
			indices[k + 1] = i * n + j + 1;
			indices[k + 2] = (i + 1) * n + j;
			
			indices[k + 3] = (i + 1) * n + j;
			indices[k + 4] = i * n + j + 1;
			indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	return Mesh(vertices, indices);
}

Mesh BasicGeometryGenerator::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	Vertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, 0.0f);
	Vertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);

	vertices.push_back(topVertex);

	float phiStep = glm::pi<float>() / stackCount;
	float thetaStep = 2.0f * glm::pi<float>() / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (uint32_t i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (uint32_t j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			Vertex v;

			// spherical to cartesian
			v.position.x = radius * sinf(phi) * sinf(theta);
			v.position.y = radius * cosf(phi);
			v.position.z = radius * sinf(phi) * cosf(theta);

			// Partial derivative of P with respect to theta
			v.tangent.x = +radius * sinf(phi) * cosf(theta);
			v.tangent.y = 0.0f;
			v.tangent.z = -radius * sinf(phi) * sinf(theta);

			v.tangent = glm::normalize(v.tangent);

			v.normal = glm::normalize(v.position);

			v.texCoord.x = theta / (2 * glm::pi<float>());
			v.texCoord.y = phi / glm::pi<float>();

			vertices.push_back(v);
		}
	}

	vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (uint32_t i = 1; i <= sliceCount; ++i)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	uint32_t baseIndex = 1;
	uint32_t ringVertexCount = sliceCount + 1;
	for (uint32_t i = 0; i < stackCount - 2; ++i)
	{
		for (uint32_t j = 0; j < sliceCount; ++j)
		{
			indices.push_back(baseIndex + i * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32_t southPoleIndex = static_cast<uint32_t>(vertices.size() - 1);

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32_t i = 0; i < sliceCount; ++i)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	return Mesh(vertices, indices);
}

Mesh BasicGeometryGenerator::CreateTerrain(const unsigned char* heightValues,
	int width, int height, int nChannels)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// vertex generation
	float yScale = 64.0f / 256.0f, yShift = 16.0f; // apply a scale + shift to the height data
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			// retrieve texel for (i, j) tex coord
			const unsigned char* texel = heightValues + (j + width * i) * nChannels;
			unsigned char y = texel[0];

			// vertex
			Vertex vertex;

			vertex.position.x = (-height / 2.0f + i);
			vertex.position.y = static_cast<float>(y) * yScale - yShift;
			vertex.position.z = (-width / 2.0f + j);

			vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			vertex.texCoord = glm::vec2(0.0f, 0.0f);
			vertex.tangent = glm::vec3(0.0f, 0.0f, 0.0f);

			vertices.push_back(vertex);
		}
	}

	// index generation
	for (int i = 0; i < height - 1; i++) // for each row a.k.a. each strip
	{
		for (int j = 0; j < width; j++) // for each column
		{
			for (int k = 0; k < 2; k++) // for each side of the strip
			{
				indices.push_back(static_cast<UINT>(j + width * (i + k)));
			}
		}
	}

	return Mesh(vertices, indices);
}

Mesh BasicGeometryGenerator::CreateTerrainPatches(int width, int height, uint32_t countOfPatches)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	vertices.reserve((countOfPatches + 1) * (countOfPatches + 1));
	indices.reserve(countOfPatches * countOfPatches);

	// vertex generation
	for (uint32_t i = 0; i <= countOfPatches; i++)
	{
		for (uint32_t j = 0; j <= countOfPatches; j++)
		{
			// vertex
			Vertex vertex;

			vertex.position.x = (-width / 2.0f + width * i / static_cast<float>(countOfPatches));
			vertex.position.y = 0.0f;
			vertex.position.z = (-height / 2.0f + height * j / static_cast<float>(countOfPatches));

			vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);

			vertex.texCoord.x = (i / static_cast<float>(countOfPatches));
			vertex.texCoord.y = (j / static_cast<float>(countOfPatches));

			vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);

			vertices.push_back(vertex);
		}
	}

	// index generation
	for (uint32_t i = 0; i <= countOfPatches - 1; i++) // for each row a.k.a. each strip
	{
		for (uint32_t j = 0; j <= countOfPatches - 1; j++) // for each column
		{
			indices.push_back(static_cast<UINT>(j + i * (countOfPatches + 1)));
			indices.push_back(static_cast<UINT>(j + (i + 1) * (countOfPatches + 1)));
			indices.push_back(static_cast<UINT>(j + 1 + i * (countOfPatches + 1)));
			indices.push_back(static_cast<UINT>(j + 1 + (i + 1) * (countOfPatches + 1)));
		}
	}

	return Mesh(vertices, indices, GL_PATCHES);
}

Mesh BasicGeometryGenerator::CreateQuad(float x, float y, float w, float h, float depth)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	vertices.resize(4);
	indices.resize(6);

	// Position coordinates specified in NDC space.
	vertices[0] = Vertex(
		x, y + h, depth,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f, 0.0f);

	vertices[1] = Vertex(
		x, y, depth,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f, 0.0f);

	vertices[2] = Vertex(
		x + w, y, depth,
		0.0f, 0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f, 0.0f);

	vertices[3] = Vertex(
		x + w, y + h, depth,
		0.0f, 0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f, 0.0f);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;

	return Mesh(vertices, indices);
}