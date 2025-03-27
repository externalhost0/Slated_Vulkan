//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "IResource.h"

#include "volk.h"
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
namespace Slate {
	struct ShaderResource : public IResource {
	public:
		void CompileToSpirv();
		void CreateVkModule(VkDevice device);
		void DestroyVkModule(VkDevice device);

		VkShaderModule GetModule() const { return this->vkModule; }
	private:
		Slang::ComPtr<ISlangBlob> spirv_code = nullptr;
		VkShaderModule vkModule = VK_NULL_HANDLE;


		VkDescriptorSet vkUniformDescriptorSet;
	private:
		void reflectLayout(slang::ProgramLayout* layout);
		Result LoadResourceImpl(const std::filesystem::path& path) override;
	};
}