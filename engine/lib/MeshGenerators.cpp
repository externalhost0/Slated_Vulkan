//
// Created by Hayden Rivas on 1/27/25.
//
#include <vector>

#include "Slate/VK/vktypes.h"
namespace Slate {
	std::vector<Vertex> GenerateGridVertices(float size, unsigned int numLines) {
		std::vector<Vertex> vertices; // tempy for return and loop
		// auto calc spacing
		float spacing = size / static_cast<float>(numLines);
		// lines along x and z
		for (unsigned int i = 0; i <= numLines; ++i) {
			float pos = -size / 2.0f + i * spacing;  // line position

			// x-axis
			Vertex xPoint;
			xPoint.color = { 1.f, 1.f, 1.f};

			xPoint.position.x = (-size / 2.0f);
			xPoint.position.y = (0.0f);
			xPoint.position.z = (pos);
			vertices.push_back(xPoint);

			Vertex yPoint;
			yPoint.color = { 1.f, 1.f, 1.f};

			yPoint.position.x = (size / 2.0f);
			yPoint.position.y = 0.0f;
			yPoint.position.z = pos;
			vertices.push_back(yPoint);

			Vertex zPoint;
			zPoint.color = { 1.f, 1.f, 1.f};

			// z-axis
			zPoint.position.x = pos;
			zPoint.position.y = 0.0f;
			zPoint.position.z = -size / 2.0f;
			vertices.push_back(zPoint);

			Vertex extraPoint;
			extraPoint.color = { 1.f, 1.f, 1.f};

			extraPoint.position.x = pos;
			extraPoint.position.y = 0.0f;
			extraPoint.position.z = size / 2.0f;
			vertices.push_back(extraPoint);
		}
		return vertices;
	}
}