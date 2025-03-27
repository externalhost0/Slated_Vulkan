//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "IResource.h"

namespace Slate {
	struct MeshResource : public IResource {
	public:
		unsigned int GetVertexCount() const { return this->vertexCount; }
		unsigned int GetIndexCount() const { return this->indexCount; }
	private:
		unsigned int vertexCount;
		unsigned int indexCount;
	private:
		Result LoadResourceImpl(const std::filesystem::path& path) override;
	};
}