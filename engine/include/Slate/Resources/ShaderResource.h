//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "IResource.h"
#include "Slate/VK/vktypes.h"
#include <vector>

#include <volk.h>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <vk_mem_alloc.h>

namespace Slate {
	struct ShaderCursor {
	public:
		void write(const void* data, size_t size) {
//			buffer.writeToBuffer(this->_allocator, &data, size, 0);
		}

		ShaderCursor field(const char* name) {
			return field(_typeLayout->findFieldIndexByName(name));
		}
		ShaderCursor field(unsigned int index) {
			slang::VariableLayoutReflection* field = _typeLayout->getFieldByIndex(index);

			ShaderCursor result = *this;
			result._typeLayout = field->getTypeLayout();
			result._byteOffset += field->getOffset();
			result._bindingIndex += field->getOffset(slang::ParameterCategory::DescriptorTableSlot);

			return result;
		}
	private:

		VkBuffer        _buffer;
		std::byte*      _bufferData;
		size_t			_byteOffset;
		VkDescriptorSet	_descriptorSet;
		uint32_t		_bindingIndex;
		uint32_t 		_bindingArrayIndex;

		slang::TypeLayoutReflection* _typeLayout;

		VmaAllocator _allocator;
	};

	struct ShaderResource : public IResource {
	public:
		void CompileToSpirv();
		void CreateVkModule(VkDevice device);
		void DestroyVkModule(VkDevice device);

		VkShaderModule GetModule() const { return this->vkModule; }
	private:
		Slang::ComPtr<ISlangBlob> spirv_code = nullptr;
		VkShaderModule vkModule = VK_NULL_HANDLE;


		std::vector<ShaderCursor> cursors;
	private:
		void reflectLayout(slang::ProgramLayout* layout);
		Result LoadResourceImpl(const std::filesystem::path& path) override;
	};

}