//
// Created by Hayden Rivas on 3/29/25.
//

#include "Slate/Systems/ShaderSystem.h"
#include "Slate/Resources/ShaderResource.h"

#include "Slate/Common/Debug.h"
#include "Slate/Common/Logger.h"
#include "Slate/SmartPointers.h"

#include <slang/slang.h>
namespace Slate {
	void ShaderSystem::onStart(Scene& scene) {

	}
	void ShaderSystem::onUpdate(Scene& scene) {

	}
	void ShaderSystem::onStop(Scene& scene) {

	}


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
				case SlangResourceShape::SLANG_TEXTURE_CUBE: return ShaderType::TextureCube;
				default: return ShaderType::Unknown;
			}
		} else if (kind == slang::TypeReflection::Kind::Struct) {
			return ShaderType::Struct;
		} else if (kind == slang::TypeReflection::Kind::Pointer) {
			return ShaderType::Pointer;
		}
		return ShaderType::Unknown;
	}
	std::string StringFromShaderType(ShaderType type) {
		switch (type) {
			case ShaderType::Struct:
				return "Struct";
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
			case ShaderType::TextureCube:
				return "TextureCube";
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
			case ShaderType::Pointer:
				return "Pointer";
			default:
				return "Unknown";
		}
	}

	void ShaderSystem::RegisterShader(ShaderResource& resource) {
		if (!this->CanRegister()) {
			LOG_USER(LogType::Warning, "Unable to register shader: {}", resource.GetFilepath());
			return;
		}
//		resource.CreateVkModule();
		resource._compileToSpirv();
		ReflectProgram(resource);
		this->resource_map[this->num_registered_shaders] = CreateStrongPtr<ShaderResource>(resource);
		this->num_registered_shaders++;
	}

	void ShaderSystem::ReflectProgram(Slate::ShaderResource& resource) {
		unsigned int count = resource._programLayout->getParameterCount();
		for (unsigned int i = 0; i < count; ++i) {
			slang::VariableLayoutReflection* var_layout = resource._programLayout->getParameterByIndex(i);

			Uniform uniform;
			uniform.type = TypefromSlangType(var_layout->getType());
			uniform.name = var_layout->getName();
			uniform.size = var_layout->getTypeLayout()->getSize();
			uniform.offset = var_layout->getOffset();
//			resource.uniforms.push_back(uniform);
		}

	}


	bool ShaderSystem::CanRegister() {
		return this->resource_map.size() < MAX_SHADER_NUM;
	}

}