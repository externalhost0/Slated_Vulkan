//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "IResource.h"
#include "Slate/Common/FastSTD.h"
#include "Slate/Common/Handles.h"
#include "Slate/VkObjects.h"

#include <array>
#include <volk.h>

namespace Slate {

	class MeshData final {
	public:
		const BufferHandle& getVertexBufferHandle() const { return _vertexBuffer; }
		const BufferHandle& getIndexBufferHandle() const { return _indexBuffer; }
		uint32_t getVertexCount() const { return _vertexCount; }
		uint32_t getIndexCount() const { return _indexCount; }
	private:
		BufferHandle _indexBuffer;
		BufferHandle _vertexBuffer;
		VkDeviceAddress _vertexBufferAddress = 0;
		uint32_t _vertexCount = 0;
		uint32_t _indexCount = 0;

		// whereever the meshdata is built
		friend class GX;
	};
	// either an editor primitive or a assets single shape
	class MeshBuffer {
	public:
		MeshBuffer() = default;
		~MeshBuffer() = default;

		bool IsIndexed() const { return indexCount > 0; }
		const AllocatedBuffer& GetIndexBuffer() const { return this->indexBuffer; }
		const VkDeviceAddress& GetVBA() const { return this->vertexBufferAddress; }
		uint32_t GetVertexCount() const { return this->vertexCount; }
		uint32_t GetIndexCount() const { return this->indexCount; }
	private:
		AllocatedBuffer indexBuffer;
		VkDeviceAddress vertexBufferAddress = 0;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
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