//
// Created by Hayden Rivas on 1/16/25.
//
#pragma once
#include "Slate/VK/vktypes.h"
#include <vector>

namespace Slate::Primitives {

	const std::vector<Vertex_Standard> quadVertices = {
			{{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // top left
			{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // bottom left
			{{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // bottom right
			{{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}  // top right
	};
	const std::vector<Vertex_Standard> planeVertices = {
			{{-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}, // top left
			{{-1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
			{{ 1.0f, 0.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
			{{ 1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}  // top right
	};

	const std::vector<uint32_t> quadIndices = {
			0, 1, 3,
			1, 2, 3
	};
	const std::vector<Vertex_Standard> cubeVertices = {
			// front face
			{{-1.f, -1.f, -1.f}, {0.f, 0.f, -1.f}, {0.0f, 0.0f}}, // A 0
			{{ 1.f, -1.f, -1.f}, {0.f, 0.f, -1.f}, {1.0f, 0.0f}}, // B 1
			{{ 1.f,  1.f, -1.f}, {0.f, 0.f, -1.f}, {1.0f, 1.0f}}, // C 2
			{{-1.f,  1.f, -1.f}, {0.f, 0.f, -1.f}, {0.0f, 1.0f}}, // D 3

			// back face
			{{-1.f, -1.f, 1.f}, {0.f, 0.f, 1.f}, {0.0f, 0.0f}}, // E 4
			{{ 1.f, -1.f, 1.f}, {0.f, 0.f, 1.f}, {1.0f, 0.0f}}, // F 5
			{{ 1.f,  1.f, 1.f}, {0.f, 0.f, 1.f}, {1.0f, 1.0f}}, // G 6
			{{-1.f,  1.f, 1.f}, {0.f, 0.f, 1.f}, {0.0f, 1.0f}}, // H 7

			// left face
			{{-1.f,  1.f, -1.f}, {-1.f, 0.f, 0.f}, {0.0f, 1.0f}}, // D 8
			{{-1.f, -1.f, -1.f}, {-1.f, 0.f, 0.f}, {0.0f, 0.0f}}, // A 9
			{{-1.f, -1.f,  1.f}, {-1.f, 0.f, 0.f}, {1.0f, 0.0f}}, // E 10
			{{-1.f,  1.f,  1.f}, {-1.f, 0.f, 0.f}, {1.0f, 1.0f}}, // H 11

			// right face
			{{1.f, -1.f, -1.f}, {1.f, 0.f, 0.f}, {0.0f, 0.0f}}, // B 12
			{{1.f,  1.f, -1.f}, {1.f, 0.f, 0.f}, {0.0f, 1.0f}}, // C 13
			{{1.f,  1.f,  1.f}, {1.f, 0.f, 0.f}, {1.0f, 1.0f}}, // G 14
			{{1.f, -1.f,  1.f}, {1.f, 0.f, 0.f}, {1.0f, 0.0f}}, // F 15

			// bottom face
			{{-1.f, -1.f, -1.f}, {0.f, -1.f, 0.f}, {0.0f, 1.0f}}, // A 16
			{{ 1.f, -1.f, -1.f}, {0.f, -1.f, 0.f}, {1.0f, 1.0f}}, // B 17
			{{ 1.f, -1.f,  1.f}, {0.f, -1.f, 0.f}, {1.0f, 0.0f}}, // F 18
			{{-1.f, -1.f,  1.f}, {0.f, -1.f, 0.f}, {0.0f, 0.0f}}, // E 19

			// top face
			{{ 1.f, 1.f, -1.f}, {0.f, 1.f, 0.f}, {1.0f, 1.0f}}, // C 20
			{{-1.f, 1.f, -1.f}, {0.f, 1.f, 0.f}, {0.0f, 1.0f}}, // D 21
			{{-1.f, 1.f,  1.f}, {0.f, 1.f, 0.f}, {0.0f, 0.0f}}, // H 22
			{{ 1.f, 1.f,  1.f}, {0.f, 1.f, 0.f}, {1.0f, 0.0f}}, // G 23
	};

	const std::vector<uint32_t> cubeIndices = {
			// front and back
			0, 3, 2,
			2, 1, 0,
			4, 5, 6,
			6, 7 ,4,
			// left and right
			11, 8, 9,
			9, 10, 11,
			12, 13, 14,
			14, 15, 12,
			// bottom and top
			16, 17, 18,
			18, 19, 16,
			20, 21, 22,
			22, 23, 20
	};

}