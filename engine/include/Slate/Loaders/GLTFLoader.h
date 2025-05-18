//
// Created by Hayden Rivas on 3/16/25.
//

#pragma once
#include "Slate/ECS/Components.h"
#include <fastgltf/core.hpp>

namespace Slate {
	class GX;

	class GLTFLoader {
	public:
		static fastgltf::Asset LoadGLTFAsset(const std::filesystem::path& path);
		static std::vector<MeshData> ProcessGLTFAsset(const fastgltf::Asset& gltf);
		static inline GX* _gx = nullptr;
	private:
		static inline fastgltf::Parser parser;
	};
}