//
// Created by Hayden Rivas on 1/30/25.
//
#include <vector>
#include <filesystem>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "Slate/Common/Debug.h"
#include "Slate/VK/vktypes.h"

#include "Slate/Loaders/GLTFLoader.h"

namespace Slate {

	// options for importing
	constexpr fastgltf::Extensions supported_extensions =
			fastgltf::Extensions::KHR_mesh_quantization      |
			fastgltf::Extensions::KHR_texture_transform      |
			fastgltf::Extensions::KHR_materials_clearcoat    |
			fastgltf::Extensions::KHR_materials_specular     |
			fastgltf::Extensions::KHR_materials_transmission |
			fastgltf::Extensions::KHR_materials_variants     |
			fastgltf::Extensions::KHR_lights_punctual;

	constexpr fastgltf::Options options =
			fastgltf::Options::DontRequireValidAssetMember |
			fastgltf::Options::LoadExternalBuffers         |
			fastgltf::Options::AllowDouble;

	fastgltf::Asset GLTFLoader::LoadGLTFAsset(const std::filesystem::path& path) {
		// get data from path
		auto data_result = fastgltf::GltfDataBuffer::FromPath(path);
		ASSERT_MSG(data_result.error() == fastgltf::Error::None, "Data load from path {} failed!", path.c_str());
		fastgltf::GltfDataBuffer data = std::move(data_result.get());

		// asset loading, requires a parser and some options
		auto asset_result = parser.loadGltf(data, path.parent_path(), options);
		ASSERT_MSG(asset_result.error() == fastgltf::Error::None, "Error when validating {}", path.c_str());
		fastgltf::Asset gltf = std::move(asset_result.get());

		return gltf;
	}

	std::vector<MeshBuffer> GLTFLoader::ProcessGLTFAsset(const fastgltf::Asset& gltf) {
		std::vector<MeshBuffer> buffer_vector = {};
		// use the same vectors for all meshes so that the memory doesnt reallocate as
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		// each mesh
		for (const fastgltf::Mesh& mesh : gltf.meshes) {
			vertices.clear();
			indices.clear();

			// each shape
			for (const fastgltf::Primitive& primitive : mesh.primitives) {
//				GeoSurface new_surface = {};
//				new_surface.count = static_cast<uint32_t>(gltf.accessors[primitive.indicesAccessor.value()].count);
//				new_surface.startIndex = static_cast<uint32_t>(indices.size());

				// LOAD INDICES
				size_t initial_vertex = vertices.size();
				{
					const fastgltf::Accessor& indexAccessor = gltf.accessors[primitive.indicesAccessor.value()];
					indices.reserve(indices.size() + indexAccessor.count);

					fastgltf::iterateAccessor<uint32_t>(gltf, indexAccessor, [&](std::uint32_t idx) {
						indices.push_back(idx + initial_vertex);
					});
				}
				// LOAD VERTICES
				{
					const fastgltf::Accessor& posAccessor = gltf.accessors[primitive.findAttribute("POSITION")->accessorIndex];
					vertices.resize(vertices.size() + posAccessor.count);

					// initialize vertex values
					// load position
					fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor, [&](glm::vec3 v, size_t index) {
						Vertex vertex = {};
						vertex.position = v;
						vertex.uv_x = 0;
						vertex.normal = glm::vec3{ 1, 0, 0 };
						vertex.uv_y = 0;
						vertex.tangent = glm::vec4{ 1.f };
						// add to the vertex vector
						vertices[initial_vertex + index] = vertex;
					});

					// load normals
					auto normals = primitive.findAttribute("NORMAL");
					if (normals != primitive.attributes.end()) {
						fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).accessorIndex], [&](glm::vec3 v, size_t index) {
							vertices[initial_vertex + index].normal = v;
						});
					}

					// load UVs
					auto uv = primitive.findAttribute("TEXCOORD_0");
					if (uv != primitive.attributes.end()) {
						fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).accessorIndex], [&](glm::vec2 v, size_t index) {
							vertices[initial_vertex + index].uv_x = v.x;
							vertices[initial_vertex + index].uv_y = v.y;
						});
					}

					// load tangents
					auto tangents = primitive.findAttribute("TANGENT");
					if (tangents != primitive.attributes.end()) {
						fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*tangents).accessorIndex], [&](glm::vec4 v, size_t index) {
							vertices[initial_vertex + index].tangent = v;
						});
					}
				}
			}
//			buffer_vector.emplace_back(pEngine->CreateMeshBuffer(vertices, indices));
		}
		return buffer_vector;

	}
}








