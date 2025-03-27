//
// Created by Hayden Rivas on 3/16/25.
//

#pragma once
#include <fastgltf/core.hpp>
#include <Slate/Components.h>

namespace Slate {
	class GLTFLoader {
	public:
		static fastgltf::Asset LoadGLTFAsset(const std::filesystem::path& path);
		static MeshAsset ProcessGLTFAsset(const fastgltf::Asset& gltf);
	private:
		static inline fastgltf::Parser parser;


	};
}