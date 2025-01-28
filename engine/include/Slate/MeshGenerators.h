//
// Created by Hayden Rivas on 1/27/25.
//

#pragma once
#include <vector>

#include "VK/vktypes.h"
namespace Slate {
	std::vector<Vertex> GenerateGridVertices(float size, unsigned int numLines);
	std::vector<Vertex> GenerateGridVertices(float size) { return GenerateGridVertices(size, static_cast<unsigned int>(size)); }
}