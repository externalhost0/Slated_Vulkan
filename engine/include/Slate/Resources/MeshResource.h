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

	// a mesh resource is only an imported mesh, not something built in
	struct MeshResource : public IResource {
	public:
		unsigned int getVertexCount() const { return this->vertexCount; }
		unsigned int getIndexCount() const { return this->indexCount; }
		unsigned int getMeshCount() const { return this->meshCount; }
		const FastVector<MeshData, 64>& getBuffers() const { return buffers; }
	private:
		unsigned int vertexCount = 0;
		unsigned int indexCount = 0;
		unsigned int meshCount = 0;
		FastVector<MeshData, 64> buffers;
	private:
		Result _loadResourceImpl(const std::filesystem::path& path) override;
	};
}