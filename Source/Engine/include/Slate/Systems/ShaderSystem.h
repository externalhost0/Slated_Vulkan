//
// Created by Hayden Rivas on 3/29/25.
//

#pragma once
#include "ISystem.h"

#include "Slate/Resources/ShaderResource.h"

#include <unordered_map>

namespace Slate {
	static constexpr unsigned int MAX_SHADER_NUM = 128;

	ShaderType TypefromSlangType(slang::TypeReflection* typeReflection);
	std::string StringFromShaderType(ShaderType type);

	class ShaderSystem : public ISystem {
	public:
		void onStart(Scene& scene) override;
		void onUpdate(Scene& scene) override;
		void onStop(Scene& scene) override;
	public:

		void RegisterShader(ShaderResource& resource);
		void UnregisterShader(StrongPtr<ShaderResource> resource);

		unsigned int GetRegisteredShaderCount() const { return this->num_registered_shaders; }
	private:
		// per shader resource operations
		void ReflectProgram(ShaderResource& resource);
	private:
		unsigned int num_registered_shaders = 0;
		std::unordered_map<uint32_t, StrongPtr<ShaderResource>> resource_map;

		bool CanRegister();
	};
}