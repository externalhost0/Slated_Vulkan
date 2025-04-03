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
		static std::vector<MeshBuffer> ProcessGLTFAsset(const fastgltf::Asset& gltf);
		static inline RenderEngine* pEngine = nullptr;
	private:
		static inline fastgltf::Parser parser;
	};
}