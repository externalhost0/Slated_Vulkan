//
// Created by Hayden Rivas on 3/16/25.
//
#include "Slate/Loaders/GLTFLoader.h"
#include "Slate/Resources/MeshResource.h"

namespace Slate {
	Result MeshResource::LoadResourceImpl(const std::filesystem::path &path) {
		auto asset = GLTFLoader::LoadGLTFAsset(path);
		auto mesh = GLTFLoader::ProcessGLTFAsset(asset);

		this->vertexCount = mesh.GetVertexCount();
		this->indexCount = mesh.GetIndexCount();

		return Result::SUCCESS;
	}
}