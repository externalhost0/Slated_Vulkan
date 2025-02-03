//
// Created by Hayden Rivas on 1/29/25.
//
#include "Slate/VK/vkdescriptor.h"
#include "Slate/Debug.h"

namespace Slate {

	// Descriptor Layout Builder //

	DescriptorLayoutBuilder& DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type) {
		VkDescriptorSetLayoutBinding newbind {};
		newbind.binding = binding;
		newbind.descriptorCount = 1;
		newbind.descriptorType = type;

		bindings.push_back(newbind);
		return *this;
	}
	void DescriptorLayoutBuilder::clear() {
		bindings.clear();
	}
	VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags) {
		for (auto& b : bindings) {
			b.stageFlags |= shaderStages;
		}

		VkDescriptorSetLayoutCreateInfo info = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		info.pNext = pNext;

		info.pBindings = bindings.data();
		info.bindingCount = (uint32_t)bindings.size();
		info.flags = flags;

		VkDescriptorSetLayout set;
		VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));
		return set;
	}


	// Descriptor Allocator Growable //

	VkDescriptorSet DescriptorAllocatorGrowable::Allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext) {
		//get or create a pool to allocate from
		VkDescriptorPool poolToUse = this->GetPool(device);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = pNext;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = poolToUse;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		VkDescriptorSet descriptor_set;
		VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &descriptor_set);

		//allocation failed. Try again
		if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {

			_fullPools.push_back(poolToUse);

			poolToUse = this->GetPool(device);
			allocInfo.descriptorPool = poolToUse;

			VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &descriptor_set));
		}

		_readyPools.push_back(poolToUse);
		return descriptor_set;
	}

	void DescriptorAllocatorGrowable::Init(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
		_ratios.clear();

		for (auto r : poolRatios) {
			_ratios.push_back(r);
		}

		VkDescriptorPool newPool = this->CreatePool(device, maxSets, poolRatios);

		_setsPerPool = static_cast<uint32_t>(maxSets * 1.5); //grow it next allocation

		_readyPools.push_back(newPool);
	}

	void DescriptorAllocatorGrowable::ClearPools(VkDevice device) {
		for (auto p : _readyPools) {
			vkResetDescriptorPool(device, p, 0);
		}
		for (auto p : _fullPools) {
			vkResetDescriptorPool(device, p, 0);
			_readyPools.push_back(p);
		}
		_fullPools.clear();
	}

	void DescriptorAllocatorGrowable::DestroyPools(VkDevice device) {
		for (auto p : _readyPools) {
			vkDestroyDescriptorPool(device, p, nullptr);
		}
		_readyPools.clear();
		for (auto p : _fullPools) {
			vkDestroyDescriptorPool(device,p,nullptr);
		}
		_fullPools.clear();
	}


	VkDescriptorPool DescriptorAllocatorGrowable::GetPool(VkDevice device) {
		VkDescriptorPool newPool;
		if (!_readyPools.empty()) {
			newPool = _readyPools.back();
			_readyPools.pop_back();
		}
		else {
			newPool = this->CreatePool(device, _setsPerPool, _ratios);

			_setsPerPool = static_cast<uint32_t>(_setsPerPool * 1.5);
			if (_setsPerPool > 4092) {
				_setsPerPool = 4092;
			}
		}

		return newPool;
	}

	VkDescriptorPool DescriptorAllocatorGrowable::CreatePool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios) {
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (PoolSizeRatio ratio : poolRatios) {
			poolSizes.push_back(VkDescriptorPoolSize{
					.type = ratio.type,
					.descriptorCount = static_cast<uint32_t>(ratio.ratio * setCount)
			});
		}

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = 0;
		pool_info.maxSets = setCount;
		pool_info.poolSizeCount = (uint32_t)poolSizes.size();
		pool_info.pPoolSizes = poolSizes.data();

		VkDescriptorPool newPool;
		vkCreateDescriptorPool(device, &pool_info, nullptr, &newPool);
		return newPool;
	}


	// Descriptor Writer //


	void DescriptorWriter::WriteBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) {
		VkDescriptorBufferInfo& info = this->bufferInfos.emplace_back(VkDescriptorBufferInfo{
				.buffer = buffer,
				.offset = offset,
				.range = size
		});

		VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstBinding = binding;
		write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pBufferInfo = &info;

		this->writes.push_back(write);
	}
	void DescriptorWriter::WriteImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type) {
		VkDescriptorImageInfo& info = this->imageInfos.emplace_back(VkDescriptorImageInfo{
				.sampler = sampler,
				.imageView = image,
				.imageLayout = layout
		});

		VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		write.dstBinding = binding;
		write.dstSet = VK_NULL_HANDLE; //left empty for now until we need to write it
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pImageInfo = &info;

		this->writes.push_back(write);
	}
	void DescriptorWriter::Clear() {
		this->imageInfos.clear();
		this->writes.clear();
		this->bufferInfos.clear();
	}
	void DescriptorWriter::UpdateSet(VkDevice device, VkDescriptorSet set) {
		for (VkWriteDescriptorSet& write : this->writes) {
			write.dstSet = set;
		}
		vkUpdateDescriptorSets(device, this->writes.size(), this->writes.data(), 0, nullptr);
	}
}