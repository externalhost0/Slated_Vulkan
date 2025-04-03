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
	enum class ShaderType : unsigned char {
		Boolean,
		Int,
		UInt,
		Float,
		Double,

		Texture2D,
		Texture3D,
		TextureCube,
		Sampler,

		Vec2,
		Vec3,
		Vec4,

		Mat2,
		Mat3,
		Mat4,

		Struct,
		Pointer,
		Unknown
	};

	struct ShaderCursor {
	public:

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

	struct Uniform {
	public:
		void write(const void* new_data) {

		};

		void* data = nullptr;
		std::string name;
		ShaderType type;
		size_t size;
		size_t offset;
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
		slang::ProgramLayout* programLayout = nullptr;
		std::vector<Uniform> uniforms;
	private:
		Result LoadResourceImpl(const std::filesystem::path& path) override;

		friend class ShaderSystem;
	};

}