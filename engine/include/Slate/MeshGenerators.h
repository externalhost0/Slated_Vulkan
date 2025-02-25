//
// Created by Hayden Rivas on 1/27/25.
//

#pragma once
#include <vector>
#include "VK/vktypes.h"
namespace Slate {
	// proper mesh generators
	void GenerateSphere(std::vector<Vertex_Standard>& vertices, std::vector<uint32_t>& indices, float radius, int stacks, int slices);

	// editor only mesh generators
	void GenerateGrid(std::vector<Vertex_Standard>& vertices, float size, unsigned int numLines);
	inline void GenerateGrid(std::vector<Vertex_Standard>& vertices, float size) { return GenerateGrid(vertices, size, static_cast<unsigned int>(size)); }
	void GenerateSimpleSphere(std::vector<Vertex_Standard>& vertices, unsigned int numSegments);
	void GenerateSpot(std::vector<Vertex_Standard>& vertices, unsigned int numSegments);
	void GenerateArrow2DMesh(std::vector<Vertex_Standard>& vertices, float shaftLength, float shaftWidth, float tipWidth, float tipHeight);
}