//
// Created by Hayden Rivas on 3/16/25.
//
#include "Slate/Debug.h"
#include "Slate/Filesystem.h"
#include "Slate/RenderEngine.h"
#include "Slate/Loaders/ShaderLoader.h"
#include "Slate/Resources/ShaderResource.h"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

namespace Slate {
	Result ShaderResource::LoadResourceImpl(const std::filesystem::path &path) {

		return Result::SUCCESS;
	}
	void ShaderResource::CompileToSpirv() {
		const std::string location = this->GetFilepath();
		// MODULE CREATION #1
		Slang::ComPtr<slang::IModule> module;
		std::vector<Slang::ComPtr<slang::IModule>> module_dependencies;
		{
			Slang::ComPtr<ISlangBlob> diagnosticsBlob;
			module = ShaderLoader::GetSession()->loadModule(location.c_str(), diagnosticsBlob.writeRef());
			EXPECT(not !module, "Failed Slang module creation!") // this is so stupid scuffed
		}


		// COMPOSITION #2
		std::array<slang::IComponentType*, 1> components = { module };
		Slang::ComPtr<slang::IComponentType> composedProgram;
		{
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = ShaderLoader::GetSession()->createCompositeComponentType(components.data(), components.size(), composedProgram.writeRef(), diagnosticsBlob.writeRef());
			EXPECT(result == SLANG_OK, "Composition failed!")
		}

		// what we can use for reflection
		// if it has INCLUDED defined, DONT reflect
		const std::string content = Filesystem::ReadTextFile(location);
		if (content.find("#define NOREFLECT") == std::string::npos) {
			this->programLayout = composedProgram->getLayout();
		}

		// LINKING #3
		Slang::ComPtr<slang::IComponentType> linkedProgram;
		{
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
			EXPECT(result == SLANG_OK, "Linking failed!");
		}

		// RETRIEVE CODE #4
		Slang::ComPtr<slang::IBlob> spirv_blob;
		{
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = linkedProgram->getTargetCode(0, spirv_blob.writeRef(), diagnosticsBlob.writeRef());
			EXPECT(result == SLANG_OK, "Entry Point retrieval failed!");
		}
		this->spirv_code = spirv_blob.get();
	}
	void ShaderResource::CreateVkModule(VkDevice device) {
		VkShaderModuleCreateInfo create_info = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		create_info.pCode = static_cast<uint32_t const*>(spirv_code->getBufferPointer());
		create_info.codeSize = spirv_code->getBufferSize();
		VkShaderModule shaderModule;
		VK_CHECK(vkCreateShaderModule(device, &create_info, nullptr, &shaderModule));
		// remember shader files shoudl include both vert and frag stages so we can assign both to one
		this->vkModule = shaderModule;
	}
	void ShaderResource::DestroyVkModule(VkDevice device) {
		vkDestroyShaderModule(device, this->vkModule, nullptr);
		this->vkModule = VK_NULL_HANDLE;
	}

}