//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "IResource.h"
#include "Slate/Common/Handles.h"
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

	struct Uniform {
	public:
		void* data = nullptr;
		std::string name;
		ShaderType type;
		std::string size;
		size_t offset;
	};

	struct ShaderResource : public IResource {
	public:
		void assignHandle(InternalShaderHandle handle);
		inline Slang::ComPtr<ISlangBlob> requestCode() {
			return _spirvCode;
		};
		inline size_t getPushSize() const {
			return _pushSize;
		}
		inline InternalShaderHandle getHandle() { return _handle; }
	private:
		void _compileToSpirv();
	private:
		bool isCompiled;
		size_t _pushSize = 0;
		InternalShaderHandle _handle; // connects with vkShaderModule
		Slang::ComPtr<ISlangBlob> _spirvCode = nullptr;
		slang::ProgramLayout*_programLayout = nullptr;
		std::vector<Uniform> _uniforms;
	private:
		Result _loadResourceImpl(const std::filesystem::path& path) override;

		friend class ShaderSystem;
	};

}