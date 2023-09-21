#pragma once
#include "Stdafx.h"
#include "Utility.h"

class Mesh;

class BasicGeometryGenerator
{
public:
	BasicGeometryGenerator() = default;

	Mesh CreateBox(float width, float height, float depth);
	Mesh CreateGrid(float width, float depth, uint32_t m, uint32_t n);
	Mesh CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);

	Mesh CreateTerrain(const unsigned char* heightValues,
		int width, int height, int nChannels);
	Mesh CreateTerrainPatches(int width, int height, uint32_t countOfPatches);

	Mesh CreateQuad(float x, float y, float w, float h, float depth);
};