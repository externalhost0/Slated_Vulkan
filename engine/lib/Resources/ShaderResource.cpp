//
// Created by Hayden Rivas on 3/16/25.
//
#include "Slate/Debug.h"
#include "Slate/Filesystem.h"
#include "Slate/Loaders/ShaderLoader.h"
#include "Slate/Resources/ShaderResource.h"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

namespace Slate {
	Result ShaderResource::LoadResourceImpl(const std::filesystem::path &path) {
		this->filepath = path;
		this->filename = path.filename();

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
			this->reflectLayout(composedProgram->getLayout());
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

	enum class ShaderType : unsigned char {
		Boolean,
		Int,
		UInt,
		Short,
		Float,
		Double,

		Texture2D,
		Texture3D,
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

	ShaderType TypefromSlangType(slang::TypeReflection* typeReflection) {
		slang::TypeReflection::Kind kind = typeReflection->getKind();
		if (kind == slang::TypeReflection::Kind::Scalar) {
			switch (typeReflection->getScalarType()) {
				case slang::TypeReflection::ScalarType::Bool:
					return ShaderType::Boolean;
				case slang::TypeReflection::Int8:
				case slang::TypeReflection::Int16:
				case slang::TypeReflection::Int32:
				case slang::TypeReflection::Int64:
					return ShaderType::Int;
				case slang::TypeReflection::UInt8:
				case slang::TypeReflection::UInt16:
				case slang::TypeReflection::UInt32:
				case slang::TypeReflection::UInt64:
					return ShaderType::UInt;
				case slang::TypeReflection::Float16:
				case slang::TypeReflection::Float32:
				case slang::TypeReflection::Float64:
					return ShaderType::Float;
				default: return ShaderType::Unknown;
			}
		} else if (kind == slang::TypeReflection::Kind::Vector) {
			switch (typeReflection->getElementCount()) {
				case 2:
					return ShaderType::Vec2;
				case 3:
					return ShaderType::Vec3;
				case 4:
					return ShaderType::Vec4;
			}
		} else if (kind == slang::TypeReflection::Kind::Matrix) {
			switch (typeReflection->getColumnCount()) {
				case 2: return ShaderType::Mat2;
				case 3: return ShaderType::Mat3;
				case 4: return ShaderType::Mat4;
			}
		} else if (kind == slang::TypeReflection::Kind::Resource) {
			switch(typeReflection->getResourceShape()) {
				case SlangResourceShape::SLANG_TEXTURE_2D: return ShaderType::Texture2D;
				case SlangResourceShape::SLANG_TEXTURE_3D: return ShaderType::Texture3D;
				default: return ShaderType::Unknown;
			}
		} else if (kind == slang::TypeReflection::Kind::Struct) {
			return ShaderType::Struct;
		}
		return ShaderType::Unknown;
	}
	std::string StringFromShaderType(ShaderType type) {
		switch (type) {
			case ShaderType::Struct:
				return "Struct";
			case ShaderType::Short:
				return "Short";
			case ShaderType::Int:
				return "Int";
			case ShaderType::UInt:
				return "UInt";
			case ShaderType::Float:
				return "Float";
			case ShaderType::Boolean:
				return "Bool";
			case ShaderType::Double:
				return "Double";
			case ShaderType::Sampler:
				return "Sampler";
			case ShaderType::Texture2D:
				return "Texture2D";
			case ShaderType::Texture3D:
				return "Texture3D";
			case ShaderType::Vec2:
				return "Vec2";
			case ShaderType::Vec3:
				return "Vec3";
			case ShaderType::Vec4:
				return "Vec4";
			case ShaderType::Mat2:
				return "Mat2";
			case ShaderType::Mat3:
				return "Mat3";
			case ShaderType::Mat4:
				return "Mat4";
			default:
				return "Unknown";
		}
	}

	void ShaderResource::reflectLayout(slang::ProgramLayout* layout) {
		unsigned int param_count = layout->getParameterCount();
		for (unsigned int i = 0; i < param_count; i++) {
			slang::VariableLayoutReflection* var_layout = layout->getParameterByIndex(i);

			slang::VariableReflection* var = var_layout->getVariable();

			fmt::print("{} {}\n", var->getName(), StringFromShaderType(TypefromSlangType(var->getType())));

			
		}
	}
}