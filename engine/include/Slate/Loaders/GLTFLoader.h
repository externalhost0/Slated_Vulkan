//
// Created by Hayden Rivas on 3/16/25.
//

#pragma once
#include "Slate/ECS/Components.h"
#include <fastgltf/core.hpp>

namespace Slate {
	class GLTFLoader {
	public:
		static fastgltf::Asset LoadGLTFAsset(const std::filesystem::path& path);
		static std::vector<MeshBuffer> ProcessGLTFAsset(const fastgltf::Asset& gltf);
	private:
		static inline fastgltf::Parser parser;
	};
}