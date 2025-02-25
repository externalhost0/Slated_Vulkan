//
// Created by Hayden Rivas on 2/19/25.
//
#include "Slate/Shader.h"

namespace Slate::Util {

	ShaderType EngineTypeFromJsonType(const nlohmann::json& type) {
		if (type.value("kind", "unknown") == "vector") {
			switch (type.value("elementCount", 4)) {
				case 2: return ShaderType::Vec2;
				case 3: return ShaderType::Vec3;
				case 4: return ShaderType::Vec4;
			}
		}
		else if (type.value("kind", "unknown") == "matrix") {
			switch (type.value("rowCount", 4)) {
				case 2: return ShaderType::Mat2;
				case 3: return ShaderType::Mat3;
				case 4: return ShaderType::Mat4;
			}
		}
		else if (type.value("kind", "unknown") == "scalar") {
			std::string scalar = type.value("scalarType", "unknown");
			if (scalar == "float16") return ShaderType::Short;
			else if (scalar == "float32") return ShaderType::Float;
			else if (scalar == "float64") return ShaderType::Double;
			else if (scalar == "int32") return ShaderType::Int;
			else if (scalar == "uint32") return ShaderType::UInt;
			else if (scalar == "boolean") return ShaderType::Boolean; // pretty sure this is wrong
		} else {
			if (type.value("kind", "unknown") == "pointer") return ShaderType::Pointer;
		}
		return ShaderType::Unknown;
	}

	std::string StringFromEngineType(ShaderType type) {
		switch (type) {
			case ShaderType::Struct: return "Struct";
			case ShaderType::Short: return "Short";
			case ShaderType::Int: return "Int";
			case ShaderType::UInt: return "UInt";
			case ShaderType::Float: return "Float";
			case ShaderType::Boolean: return "Bool";
			case ShaderType::Double: return "Double";
			case ShaderType::Image: return "Image";
			case ShaderType::Sampler: return "Sampler";

			case ShaderType::Vec2: return "Vec2";
			case ShaderType::Vec3: return "Vec3";
			case ShaderType::Vec4: return "Vec4";

			case ShaderType::Mat2: return "Mat2";
			case ShaderType::Mat3: return "Mat3";
			case ShaderType::Mat4: return "Mat4";

			case ShaderType::Pointer: return "Pointer";
			default: return "Unknown";
		}
	}
	std::map<ShaderType, std::string> MappedTypeToString = {
			{ShaderType::Struct, "Struct"},
			{ShaderType::Short, "Short"},
			{ShaderType::Int, "Int"},
			{ShaderType::Float, "Float"},
			{ShaderType::Boolean, "Bool"},
			{ShaderType::Double, "Double"},

			{ShaderType::Image, "Image"},
			{ShaderType::Sampler, "Sampler"},

			{ShaderType::Vec2, "Vec2"},
			{ShaderType::Vec3, "Vec3"},
			{ShaderType::Vec4, "Vec4"},

			{ShaderType::Mat2, "Mat2"},
			{ShaderType::Mat3, "Mat3"},
			{ShaderType::Mat4, "Mat4"},

			{ShaderType::Pointer, "Pointer"},
			{ShaderType::Unknown, "Unknown"}
	};
}