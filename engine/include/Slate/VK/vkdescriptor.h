//
// Created by Hayden Rivas on 1/29/25.
//
#pragma once
#include <vector>
#include <span>
#include <deque>
#include <vulkan/vulkan.h>

namespace Slate {
	struct DescriptorLayoutBuilder {

		DescriptorLayoutBuilder& addBinding(uint32_t binding, VkDescriptorType type);
		VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);

		void clear();
	private:
		std::vector<VkDescriptorSetLayoutBinding> bindings;
	};

	struct DescriptorAllocatorGrowable {
		struct PoolSizeRatio {
			VkDescriptorType type;
			float ratio;
		};
		VkDescriptorSet Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext = nullptr);

		void Init(VkDevice device, uint32_t initial_sets, std::span<PoolSizeRatio> pool_ratios);
		void ClearPools(VkDevice device);
		void DestroyPools(VkDevice device);
	private:
		VkDescriptorPool GetPool(VkDevice device);
		VkDescriptorPool CreatePool(VkDevice device, uint32_t set_count, std::span<PoolSizeRatio> pool_ratiose);
	private:
		std::vector<PoolSizeRatio> _ratios;
		std::vector<VkDescriptorPool> _fullPools;
		std::vector<VkDescriptorPool> _readyPools;
		uint32_t _setsPerPool;
	};

	struct DescriptorWriter {
		std::deque<VkDescriptorImageInfo> imageInfos;
		std::deque<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkWriteDescriptorSet> writes;

		void WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
		void WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

		void Clear();
		void UpdateSet(VkDevice device, VkDescriptorSet set);
	};
}