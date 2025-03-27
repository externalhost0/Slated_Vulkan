//
// Created by Hayden Rivas on 1/27/25.
//
#include <vector>
#include <glm/ext/scalar_constants.hpp>

#include "Slate/VK/vktypes.h"

#include "Slate/MeshGenerators.h"
namespace Slate {
	void GenerateGridEXT(std::vector<Vertex>& vertices, float size, unsigned int numLines) {
		vertices.clear();
		// auto calc spacing
		float spacing = size / static_cast<float>(numLines);
		// lines along x and z
		for (unsigned int i = 0; i <= numLines; ++i) {
			float pos = -size / 2.0f + i * spacing;  // line position

			// x-axis
			Vertex xPoint {};
			xPoint.position.x = (-size / 2.0f);
			xPoint.position.y = (0.0f);
			xPoint.position.z = (pos);
			vertices.push_back(xPoint);

			Vertex yPoint {};
			yPoint.position.x = (size / 2.0f);
			yPoint.position.y = 0.0f;
			yPoint.position.z = pos;
			vertices.push_back(yPoint);

			// z-axis
			Vertex zPoint {};
			zPoint.position.x = pos;
			zPoint.position.y = 0.0f;
			zPoint.position.z = -size / 2.0f;
			vertices.push_back(zPoint);

			// loop back point
			Vertex extraPoint {};
			extraPoint.position.x = pos;
			extraPoint.position.y = 0.0f;
			extraPoint.position.z = size / 2.0f;
			vertices.push_back(extraPoint);
		}
	}
	void GenerateGrid(std::vector<Vertex>& vertices, float size, unsigned int numLines) {
		vertices.clear();
		float spacing = size / static_cast<float>(numLines);
		float halfSize = size / 2.0f;

		for (unsigned int i = 0; i < numLines; ++i) {
			float z = -halfSize + i * spacing;

			if (i % 2 == 0) {
				// left to right
				vertices.push_back({ {-halfSize, 0.0f, z} });
				vertices.push_back({ {halfSize, 0.0f, z} });
			} else {
				// right to left (trace back)
				vertices.push_back({ {halfSize, 0.0f, z} });
				(vertices.push_back({ {-halfSize, 0.0f, z} }));
			}
		}

		for (unsigned int i = 0; i < numLines+1; ++i) {
			float x = -halfSize + i * spacing;

			if (i % 2 == 0) {
				// bottom to top
				vertices.push_back({ {x, 0.0f, -halfSize} });
				vertices.push_back({ {x, 0.0f, halfSize} });
			} else {
				// top to bottom (trace back)
				vertices.push_back({ {x, 0.0f, halfSize} });
				vertices.push_back({ {x, 0.0f, -halfSize} });
			}
		}
		vertices.push_back({ { -halfSize, 0.0f, halfSize }});
	}

	void GenerateSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius, int stacks, int slices) {
		vertices.clear();
		indices.clear();

		// vertices
		for (int i = 0; i <= stacks; ++i) {
			float phi = glm::pi<float>() * i / stacks;  // Latitude angle [0, pi]

			for (int j = 0; j <= slices; ++j) {
				float theta = 2.0f * glm::pi<float>() * j / slices; // Longitude angle [0, 2pi]

				glm::vec3 pos = {
						radius * sin(phi) * cos(theta), // x
						radius * cos(phi),             // y
						radius * sin(phi) * sin(theta)  // z
				};

				glm::vec3 normal = glm::normalize(pos);
				glm::vec2 uv = { (float)j / slices, (float)i / stacks };

				vertices.push_back({ pos, normal, uv });
			}
		}
		// indices
		for (int i = 0; i < stacks; ++i) {
			for (int j = 0; j < slices; ++j) {
				int first = i * (slices + 1) + j;
				int second = first + slices + 1;

				// first triangle
				indices.push_back(first);
				indices.push_back(first + 1);
				indices.push_back(second);

				// second triangle
				indices.push_back(second);
				indices.push_back(first + 1);
				indices.push_back(second + 1);
			}
		}
	}

	void GenerateSimpleSphere(std::vector<Vertex>& vertices, unsigned int numSegments) {
		vertices.clear();
		float radius = 1.0f;

		// require we have even segments
		if (numSegments%2!=0) {
			numSegments+=1;
		}

		float resolutionIncrement = 2.0f * (float) M_PI / (float) numSegments;

		// first vertical
		for (int i = 0; i < numSegments; i++) {
			float increment = (float)i * resolutionIncrement;
			float x = radius * cos(increment);
			float y = radius * sin(increment);

			Vertex point {};
			point.position = {x, y, 0.f};
			vertices.push_back(point);
		}
		// first horizontal
		for (int i = 0; i < numSegments+(numSegments/4); i++) {
			float increment = (float)i * resolutionIncrement;
			float x = radius * cos(increment);
			float z = radius * sin(increment);

			Vertex point {};
			point.position = {x, 0.f, z};
			vertices.push_back(point);
		}
		// second vertical
		for (int i = 0; i < numSegments+(numSegments/4); i++) {
			float increment = (float)i * resolutionIncrement;
			float x = radius * cos(increment);
			float y = radius * sin(increment);

			Vertex point {};
			point.position = {0.f, y, x};
			vertices.push_back(point);
		}
	}

	void GenerateSpot(std::vector<Vertex>& vertices, unsigned int numSegments) {
		vertices.clear();

		float radius = 0.5f;
		float height = radius;

		float angleIncrement = 2.0f * static_cast<float>(M_PI) / static_cast<float>(numSegments);

		std::vector<Vertex> baseVertices;
		for (unsigned int i = 0; i < numSegments; ++i) {
			float angle = i * angleIncrement;
			float x = radius * cos(angle);
			float z = radius * sin(angle);

			Vertex point{};
			point.position = {x, -height, z};
			baseVertices.push_back(point);
		}

		// add tip
		Vertex apex{};
		apex.position = {0.f, 0.f, 0.f};
		vertices.push_back(apex);
		unsigned int apexIndex = 0;

		// base circle
		unsigned int baseStartIndex = vertices.size();
		vertices.insert(vertices.end(), baseVertices.begin(), baseVertices.end());

		// sides
		for (unsigned int i = 0; i < numSegments; ++i) {
			unsigned int nextIndex = (i + 1) % numSegments + baseStartIndex;

			vertices.push_back(vertices[apexIndex]);
			vertices.push_back(vertices[i + baseStartIndex]);
			vertices.push_back(vertices[nextIndex]);
		}
	}


	void GenerateArrow2DMesh(std::vector<Vertex>& vertices, float shaftLength, float shaftWidth, float tipWidth, float tipHeight) {
		vertices.clear();
		// bottom vertex
		{
			Vertex point{};
			point.position = {0.f, 0.f, 0.f};
			vertices.push_back(point);
		}

		// top vertex
		{
			Vertex point{};
			point.position = {0.f, -shaftLength, 0.f};
			vertices.push_back(point);
		}

		// triangle tip
		{
			Vertex point{};
			point.position = {-tipWidth, -shaftLength, 0.f};
			vertices.push_back(point);
		}

		// triangle tip
		{
			Vertex point{};
			point.position = {0.f, -shaftLength - tipHeight, 0.f};
			vertices.push_back(point);
		}

		{
			Vertex point{};
			point.position = {tipWidth, -shaftLength, 0.f};
			vertices.push_back(point);
		}

		// and back to top vertex
		{
			Vertex point{};
			point.position = {0.f, -shaftLength, 0.f};
			vertices.push_back(point);
		}

		// now again but on the z axis so we have two arrow tips!
		{
			Vertex point{};
			point.position = {0.f, -shaftLength, -tipWidth};
			vertices.push_back(point);
		}

		{
			Vertex point{};
			point.position = {0.f, -shaftLength - tipHeight, 0.f};
			vertices.push_back(point);
		}

		{
			Vertex point{};
			point.position = {0.f, -shaftLength, tipWidth};
			vertices.push_back(point);
		}

		// and back again to top vertex
		{
			Vertex point{};
			point.position = {0.f, -shaftLength, 0.f};
			vertices.push_back(point);
		}
	}
}