//
// Created by Hayden Rivas on 3/23/25.
//
#include "Slate/RenderEngine.h"
#include "Slate/Debug.h"
#include "Slate/VK/vkutil.h"
#include "Slate/Components.h"
#include "Slate/Filesystem.h"

#define VOLK_IMPLEMENTATION
#include <volk.h>
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_IMPLEMENTATION
#define VMA_LEAK_LOG_FORMAT(format, ...) {  \
    printf((format), __VA_ARGS__);          \
    printf("\n");           			    \
}
#include <vk_mem_alloc.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace Slate {
	void RenderEngine::Immediate_Submit(std::function<void(VkCommandBuffer cmd)> &&function) {
		VK_CHECK(vkResetFences(_vkDevice, 1, &_immFence));
		VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));

		VkCommandBufferBeginInfo cmdBeginInfo = vkinfo::CreateCommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VK_CHECK(vkBeginCommandBuffer(_immCommandBuffer, &cmdBeginInfo));

		function(_immCommandBuffer);

		VK_CHECK(vkEndCommandBuffer(_immCommandBuffer));

		VkCommandBufferSubmitInfo cmdinfo = vkinfo::CreateCommandBufferSubmitInfo(_immCommandBuffer);
		VkSubmitInfo2 submit = vkinfo::CreateSubmitInfo(&cmdinfo, nullptr, nullptr);

		// submit command buffer to the queue and execute it.
		//  _renderFence will now block until the graphic commands finish execution
		VK_CHECK(vkQueueSubmit2KHR(_vkGraphicsQueue, 1, &submit, _immFence));

		VK_CHECK(vkWaitForFences(_vkDevice, 1, &_immFence, true, 9999999999));
	}
	vktypes::AllocatedBuffer RenderEngine::CreateBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags) const {
		VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.pNext = nullptr;
		bufferInfo.size = allocSize;
		bufferInfo.usage = usage;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = memoryUsage;
		vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | flags;

		vktypes::AllocatedBuffer newBuffer = {};
		newBuffer.minOffsetAlignment = this->_vkPhysDeviceProperties.limits.minUniformBufferOffsetAlignment;
		VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.allocationInfo));
		return newBuffer;
	}
	vktypes::AllocatedImage RenderEngine::CreateImage(VkExtent2D extent, VkFormat format, VkImageUsageFlags usages, VkSampleCountFlags samples, bool mipmapped) const {
		VkExtent3D extent3D = {
				.width = extent.width,
				.height = extent.height,
				.depth = 1
		};
		return this->CreateImage(extent3D, format, usages, samples, mipmapped);
	}
	vktypes::AllocatedImage RenderEngine::CreateImage(VkExtent3D extent, VkFormat format, VkImageUsageFlags usages, VkSampleCountFlags samples, bool mipmapped) const {
		vktypes::AllocatedImage newImage = {};
		newImage.imageFormat = format;
		newImage.imageExtent = extent;

		// small checking if mipmapping was true, than so some fancy math to get the levels we need
		uint32_t miplevels = 1;
		if (mipmapped) miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
		VkImageCreateInfo img_info = vkinfo::CreateImageInfo(extent, format, usages, miplevels, samples);

		VmaAllocationCreateInfo allocinfo = {};
		allocinfo.usage = VMA_MEMORY_USAGE_AUTO; // was gpu only mem but changed to auto
		allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK(vmaCreateImage(this->_allocator, &img_info, &allocinfo, &newImage.image, &newImage.allocation, &newImage.allocationInfo));

		VkImageViewCreateInfo view_info = vkinfo::CreateImageViewInfo(format, newImage.image, vkutil::AspectMaskFromFormat(format));
		view_info.subresourceRange.levelCount = img_info.mipLevels;
		VK_CHECK(vkCreateImageView(this->_vkDevice, &view_info, nullptr, &newImage.imageView));

		return newImage;
	}
	vktypes::AllocatedImage RenderEngine::CreateImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
		size_t data_size = size.depth * size.width * size.height * 4;
		vktypes::AllocatedBuffer uploadbuffer = this->CreateBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		memcpy(uploadbuffer.allocationInfo.pMappedData, data, data_size);

		vktypes::AllocatedImage new_image = this->CreateImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_SAMPLE_COUNT_1_BIT, mipmapped);

		this->Immediate_Submit([&](VkCommandBuffer cmd) {
			vkutil::TransitionImageLayout(cmd, new_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

			VkBufferImageCopy copyRegion = {};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageExtent = size;

			// copy the buffer into the image
			vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			if (mipmapped) {
				vkutil::GenerateMipmaps(cmd, new_image.image, VkExtent2D{new_image.imageExtent.width,new_image.imageExtent.height});
			} else {
				vkutil::TransitionImageLayout(cmd, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		});
		this->DestroyBuffer(uploadbuffer);
		return new_image;
	}
	vktypes::AllocatedImage RenderEngine::CreateImage(const std::filesystem::path& file_path, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
		vktypes::AllocatedImage newImage = {};

		int width, height, nrChannels;
		void* data = stbi_load(file_path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		VkExtent3D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		if (data) {
			newImage = this->CreateImage(data, extent, format, usage, mipmapped);
		} else {
			fmt::print(stderr, "Failed to load image\n");
		}
		stbi_image_free(data);

		return newImage;
	}
	MeshBuffer RenderEngine::CreateMeshBuffer(std::vector<Vertex> vertices) {
		// just calculate sizes of our parameters
		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);

		MeshBuffer mesh = {};
		mesh.vertexCount = vertices.size();

		vktypes::StagingMeshBuffers surfaceBuffer = {};
		// create vertex buffer
		surfaceBuffer.vertexBuffer = this->CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		//find the adress of the vertex buffer
		VkBufferDeviceAddressInfo deviceAdressInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = nullptr };
		deviceAdressInfo.buffer = surfaceBuffer.vertexBuffer.buffer;
		mesh.vertexBufferAddress = vkGetBufferDeviceAddress(_vkDevice, &deviceAdressInfo);

		vktypes::AllocatedBuffer staging = this->CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void *data = staging.allocation->GetMappedData();

		// copy vertex buffer
		memcpy(data, vertices.data(), vertexBufferSize);

		this->Immediate_Submit([&](VkCommandBuffer cmd) {
			VkBufferCopy vertexCopy{0};
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, surfaceBuffer.vertexBuffer.buffer, 1, &vertexCopy);
		});
		this->DestroyBuffer(staging);
		return mesh;
	}
	MeshBuffer RenderEngine::CreateMeshBuffer(std::vector<Vertex> vertices, std::vector<uint32_t> indices) {
		// just calculate sizes of our parameters
		const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
		const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

		MeshBuffer mesh = {};
		mesh.indexCount = indices.size();
		vktypes::StagingMeshBuffers surfaceBuffer = {};

		// create vertex buffer
		surfaceBuffer.vertexBuffer = this->CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		//find the adress of the vertex buffer
		VkBufferDeviceAddressInfo deviceAdressInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .pNext = nullptr};
		deviceAdressInfo.buffer = surfaceBuffer.vertexBuffer.buffer;
		mesh.vertexBufferAddress = vkGetBufferDeviceAddress(this->_vkDevice, &deviceAdressInfo);

		//create index buffer
		mesh.indexBuffer = this->CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

		vktypes::AllocatedBuffer staging = this->CreateBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void *data = staging.allocation->GetMappedData();

		// copy vertex buffer
		memcpy(data, vertices.data(), vertexBufferSize);
		// copy index buffer
		memcpy((char *) data + vertexBufferSize, indices.data(), indexBufferSize);

		this->Immediate_Submit([&](VkCommandBuffer cmd) {
			VkBufferCopy vertexCopy{0};
			vertexCopy.dstOffset = 0;
			vertexCopy.srcOffset = 0;
			vertexCopy.size = vertexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, surfaceBuffer.vertexBuffer.buffer, 1, &vertexCopy);

			VkBufferCopy indexCopy{0};
			indexCopy.dstOffset = 0;
			indexCopy.srcOffset = vertexBufferSize;
			indexCopy.size = indexBufferSize;

			vkCmdCopyBuffer(cmd, staging.buffer, mesh.indexBuffer.buffer, 1, &indexCopy);
		});

		this->DestroyBuffer(staging);
		return mesh;
	}
	void RenderEngine::DestroyBuffer(vktypes::AllocatedBuffer& allocatedBuffer) const {
		vmaDestroyBuffer(this->_allocator, allocatedBuffer.buffer, allocatedBuffer.allocation);
		allocatedBuffer.allocation = VK_NULL_HANDLE;
		allocatedBuffer.buffer = VK_NULL_HANDLE;
	}
	void RenderEngine::DestroyImage(vktypes::AllocatedImage& allocatedImage) const {
		vkDestroyImageView(this->_vkDevice, allocatedImage.imageView, nullptr);
		vmaDestroyImage(this->_allocator, allocatedImage.image, allocatedImage.allocation);
		allocatedImage.allocation = VK_NULL_HANDLE;
		allocatedImage.image = VK_NULL_HANDLE;
		allocatedImage.imageView = VK_NULL_HANDLE;
		allocatedImage.imageFormat = VK_FORMAT_UNDEFINED;
	}

	// TODO::better organize this editor only thing eventually
	void RenderEngine::DrawMeshData_EXT(const MeshBuffer& data) {
		if (data.IsIndexed()) {
			vkCmdBindIndexBuffer(this->GetCurrentFrameData()._commandBuffer, data.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(this->GetCurrentFrameData()._commandBuffer, data.indexCount, 1, 0, 0, 0);
		} else {
			vkCmdDraw(this->GetCurrentFrameData()._commandBuffer, data.vertexCount, 1, 0, 0);
		}
	}

}