//
// Created by Hayden Rivas on 3/16/25.
//
#include "Slate/Resources/ShaderResource.h"
#include "Slate/Common/HelperMacros.h"
#include "Slate/Filesystem.h"
#include "Slate/Loaders/ShaderLoader.h"
#include "Slate/Systems/ShaderSystem.h"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

namespace Slate {
	void printTypeLayout(slang::TypeLayoutReflection* typeLayout);
	void printVariableLayout(slang::VariableLayoutReflection* variable);


	Result ShaderResource::_loadResourceImpl(const std::filesystem::path& path) {
		_compileToSpirv();

		return Result::SUCCESS;
	}
	std::string KindToString(slang::TypeReflection::Kind kind) {
		switch (kind) {
			case slang::TypeReflection::Kind::None: return "None";
			case slang::TypeReflection::Kind::Struct: return "Struct";
			case slang::TypeReflection::Kind::Array: return "Array";
			case slang::TypeReflection::Kind::Matrix: return "Matrix";
			case slang::TypeReflection::Kind::Vector: return "Vector";
			case slang::TypeReflection::Kind::Scalar: return "Scalar";
			case slang::TypeReflection::Kind::ConstantBuffer: return "Constant Buffer";
			case slang::TypeReflection::Kind::Resource: return "Resource";
			case slang::TypeReflection::Kind::SamplerState: return "Sampler State";
			case slang::TypeReflection::Kind::TextureBuffer: return "Texture Buffer";
			case slang::TypeReflection::Kind::ShaderStorageBuffer: return "Shader Storage Buffer";
			case slang::TypeReflection::Kind::ParameterBlock: return "Parameter Block";
			case slang::TypeReflection::Kind::Pointer: return "Pointer";
			case slang::TypeReflection::Kind::GenericTypeParameter:
			case slang::TypeReflection::Kind::Interface:
			case slang::TypeReflection::Kind::OutputStream:
			case slang::TypeReflection::Kind::Specialized:
			case slang::TypeReflection::Kind::Feedback:
			case slang::TypeReflection::Kind::DynamicResource:
				return "Unknown Kind";
		}
	}

	void ShaderResource::_compileToSpirv() {
		const std::string location = this->getFilepath();
		// MODULE CREATION #1
		Slang::ComPtr<slang::IModule> module;
		std::vector<Slang::ComPtr<slang::IModule>> module_dependencies;
		{
			Slang::ComPtr<ISlangBlob> diagnosticsBlob;
			module = ShaderLoader::GetSession()->loadModule(location.c_str(), diagnosticsBlob.writeRef());
			ASSERT_MSG(module, "Failed Slang module creation! Diagnostics Below:\n{}", (const char*)diagnosticsBlob->getBufferPointer()); // this is so stupid scuffed
		}
		// COMPOSITION #2
		std::array<slang::IComponentType*, 1> components_to_link = { module };
		Slang::ComPtr<slang::IComponentType> composedProgram;
		{
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = ShaderLoader::GetSession()->createCompositeComponentType(components_to_link.data(), components_to_link.size(), composedProgram.writeRef(), diagnosticsBlob.writeRef());
			ASSERT_MSG(result == SLANG_OK, "Composition failed! Diagnostics Below:\n{}", (const char*)diagnosticsBlob->getBufferPointer());
		}
		// LINKING #3
		Slang::ComPtr<slang::IComponentType> linkedProgram;
		{
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
			ASSERT_MSG(result == SLANG_OK, "Linking failed! Diagnostics Below:\n{}", diagnosticsBlob->getBufferPointer());
		}
		// REFLECTION

		slang::ProgramLayout* layout = linkedProgram->getLayout();
		assert(layout);
		size_t pcSize = 0;
		// total parameters in the shader, 2 should be standard, global descriptors + push constants
		unsigned int parameter_count = layout->getParameterCount();
		for (unsigned int i = 0; i < parameter_count; ++i) {
			slang::VariableLayoutReflection* variablelayout = layout->getParameterByIndex(i);
			fmt::print("Name: {}\n", variablelayout->getName());
			fmt::print("Type: {}\n", variablelayout->getTypeLayout()->getName());
			fmt::print("Element Type: {}\n", variablelayout->getTypeLayout()->getElementTypeLayout()->getName());
			fmt::print("Total Size: {}\n", variablelayout->getTypeLayout()->getElementTypeLayout()->getSize());
			if (variablelayout->getTypeLayout()->getContainerVarLayout()->getCategory() == slang::ParameterCategory::PushConstantBuffer) {
				pcSize = variablelayout->getTypeLayout()->getElementTypeLayout()->getSize();
			}
			unsigned int fieldCount = variablelayout->getTypeLayout()->getElementTypeLayout()->getFieldCount();
			for (int j = 0; j < fieldCount; ++j) {
				fmt::print("Field {}: {} - {}\n", j,
						   StringFromShaderType(TypefromSlangType(variablelayout->getTypeLayout()->getElementTypeLayout()->getFieldByIndex(j)->getType())),
						   variablelayout->getTypeLayout()->getElementTypeLayout()->getFieldByIndex(j)->getName());
			}
			fmt::print("-------------------------\n");
		}

		// RETRIEVE CODE #4
		Slang::ComPtr<slang::IBlob> spirv_blob;
		{
			Slang::ComPtr<slang::IBlob> diagnosticsBlob;
			SlangResult result = linkedProgram->getTargetCode(0, spirv_blob.writeRef(), diagnosticsBlob.writeRef());
			ASSERT_MSG(result == SLANG_OK, "Target code retrieval failed! Diagnostics Below:\n{}", diagnosticsBlob->getBufferPointer());
		}
		this->_spirvCode = spirv_blob;
		this->_pushSize = pcSize;

		// test bench, doesnt do anything
		auto s1 = sizeof(GPU::DrawPushConstants); // 80 on gpu
		auto s2 = sizeof(GPU::PushConstants_EditorImages); // 96 on gpu
		auto s3 = sizeof(GPU::PushConstants_EditorPrimitives); // 80 on gpu

//		static_assert(sizeof(GPU::DrawPushConstants) == 80, "PushConstants size mismatch!");
//		static_assert(sizeof(GPU::PushConstants_EditorImages) == 96, "PushConstants size mismatch!");
//		static_assert(sizeof(GPU::PushConstants_EditorPrimitives) == 80, "PushConstants size mismatch!");

	}
	void ShaderResource::assignHandle(InternalShaderHandle handle) {
		_handle = handle;
	}
}




















