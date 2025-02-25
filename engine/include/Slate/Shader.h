//
// Created by Hayden Rivas on 2/19/25.
//

#pragma once
#include <string>

#include <volk.h>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include <slang/slang.h>
#include <nlohmann/json.hpp>

#include "SmartPointers.h"

namespace Slate {
	enum class ShaderType : unsigned char {
		Boolean,
		Int,
		UInt,
		Short,
		Float,
		Double,

		Image,
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
	namespace Util {
		ShaderType EngineTypeFromJsonType(const nlohmann::json& type);

		std::string StringFromEngineType(ShaderType type);
	}

	// different ways data is represented inside a shader
	// needs to be defined so it can be reflected and displayed properly in editor
	struct Resource {
		uint32_t id;
		std::string name;
	};
	struct PlainType : Resource {
		ShaderType glsltype = ShaderType::Unknown;
	};
	struct Texture : Resource {

	};
	struct Sampler : Resource {

	};
	// compared to a UniformBlock, CustomTypes are just a way to represent a user defined struct in shader code
	// they do not manage shader memory retrieval/sending unlike the blocks do
	struct CustomType : Resource {
		std::string usertype;
		std::vector<std::variant<PlainType, CustomType>> members;
	};
	// uniform block can contain:
	// 1. plain types (float3, int, mat4)
	// 2. custom types (AmbientLight, PointLight)
	struct UBO : Resource {
		std::string usertype;
		std::vector<std::variant<PlainType, CustomType>> members;
	};
	struct PushConstant : Resource {

	};
	struct SSBO : Resource {

	};


	// how each bundled shader is made
	struct ShaderProgram {
		VkShaderModule vertModule = VK_NULL_HANDLE;
		VkShaderModule fragModule = VK_NULL_HANDLE;

		std::vector<PushConstant> pushConstants;
		std::vector<UBO> uniformBlockObjects;
		std::vector<SSBO> storageBufferObjects;


		// sorta optional fields
		std::string filename = "null";
		uint8_t priority = 1;

		ShaderProgram() = default;
	};
}