//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "IResource.h"
#include "Slate/FastSTD.h"

#include <array>

namespace Slate {

	// either an editor primitive or a assets single shape
	class MeshBuffer {
	public:
		MeshBuffer() = default;
		~MeshBuffer() = default;

		bool IsIndexed() const { return indexCount > 0; }
		const vktypes::AllocatedBuffer& GetIndexBuffer() const { return this->indexBuffer; }
		const VkDeviceAddress& GetVBA() const { return this->vertexBufferAddress; }
		uint32_t GetVertexCount() const { return this->vertexCount; }
		uint32_t GetIndexCount() const { return this->indexCount; }
	private:
		vktypes::AllocatedBuffer indexBuffer;
		VkDeviceAddress vertexBufferAddress = 0;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		friend class RenderEngine;
	};

	// a mesh resource is only an imported mesh, not something built in
	struct MeshResource : public IResource {
	public:
		unsigned int GetVertexCount() const { return this->vertexCount; }
		unsigned int GetIndexCount() const { return this->indexCount; }
	private:
		unsigned int vertexCount = 0;
		unsigned int indexCount = 0;
		FastVector<MeshBuffer, 64> buffers;
	private:
		Result LoadResourceImpl(const std::filesystem::path& path) override;
	};
}