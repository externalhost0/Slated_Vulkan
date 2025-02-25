//
// Created by Hayden Rivas on 1/30/25.
//
#include <vector>
#include <filesystem>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "Slate/VK/vktypes.h"
#include "Slate/Debug.h"

#include "Slate/Loader.h"
namespace Slate {

	void Loader::loadGTLF(const std::filesystem::path& file_path) {
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
				fastgltf::Options::LoadExternalBuffers |
				fastgltf::Options::AllowDouble;


		// meshData
		auto data_result = fastgltf::GltfDataBuffer::FromPath(file_path);
		EXPECT(data_result.error() != fastgltf::Error::None, "Data load from path ({}) failed!", file_path.c_str());
		fastgltf::GltfDataBuffer data = std::move(data_result.get());

		// asset loading, requires a parser and some options
		auto asset_result = parser.loadGltf(data, file_path.parent_path(), options);
		EXPECT(asset_result.error() != fastgltf::Error::None, "Error when validating {}", file_path.c_str());
		fastgltf::Asset asset = std::move(asset_result.get());


		// more specific processes below,
		// start with mesh loading

		// use the same vectors for all meshes so that the memory doesnt reallocate as
		std::vector<uint32_t> indices;
		std::vector<Vertex_Standard> vertices;


		for (fastgltf::Mesh& mesh : asset.meshes) {
			for (fastgltf::Primitive& primitive : mesh.primitives) {

			}
		}
		for (fastgltf::Sampler& sampler : asset.samplers) {

		}
		for (fastgltf::Image& image : asset.images) {

		}
		for (fastgltf::Light& light : asset.lights) {

		}
		for (fastgltf::Buffer& buffer : asset.buffers) {

		}
		for (fastgltf::Material& material : asset.materials) {

		}
	}
}