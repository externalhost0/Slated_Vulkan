//
// Created by Hayden Rivas on 3/16/25.
//
#include "Slate/Loaders/GLTFLoader.h"
#include "Slate/Resources/MeshResource.h"

namespace Slate {
	Result MeshResource::LoadResourceImpl(const std::filesystem::path &path) {
		auto asset = GLTFLoader::LoadGLTFAsset(path);
		this->buffers = GLTFLoader::ProcessGLTFAsset(asset);

		unsigned int v_count = 0, i_count = 0;
		for (const MeshBuffer& buffer : this->buffers) {
			v_count += buffer.GetVertexCount();
			i_count += buffer.GetIndexCount();
		}
		this->vertexCount = v_count;
		this->indexCount = i_count;


		return Result::SUCCESS;
	}
}