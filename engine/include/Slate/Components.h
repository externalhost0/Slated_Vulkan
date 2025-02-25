//
// Created by Hayden Rivas on 1/20/25.
//
#pragma once
#include <filesystem>
#include <string>
#include <utility>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <spirv_cross/spirv_cross.hpp>

#include "Shader.h"
#include "VK/vktypes.h"

namespace Slate {
	struct CoreComponent {
		std::string name {"Unnamed Entity"};
	};

	struct TransformComponent {
		glm::vec3 Position { 0.f };
		glm::quat Rotation { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 Scale    { 1.f };

		TransformComponent() = default;
		explicit TransformComponent(glm::vec3 pos) : Position(pos) {};
	};


	struct MeshComponent {
		vktypes::MeshData meshData;
		ShaderProgram shaderProgram;

		MeshComponent() = default;
		MeshComponent(vktypes::MeshData data, ShaderProgram program) : meshData(data), shaderProgram(std::move(program)) {};
	};




	struct MaterialComponent {

	};

	enum class LightAppearance {
		COLOR,
		TEMPERATURE
	};

	struct EnvironmentComponent {
		GPU::AmbientLight ambient;
	};
	struct DirectionalLightComponent {
		GPU::DirectionalLight directional;
		LightAppearance appearance = LightAppearance::COLOR;

		DirectionalLightComponent() = default;
		explicit DirectionalLightComponent(const glm::vec3& color) { directional.Color = color; };
	};
	struct PointLightComponent{
		GPU::PointLight point;
		LightAppearance appearance = LightAppearance::COLOR;

		PointLightComponent() = default;
		explicit PointLightComponent(const glm::vec3& color) { point.Color = color; };
	};
	struct SpotLightComponent{
		GPU::SpotLight spot;
		LightAppearance appearance = LightAppearance::COLOR;

		SpotLightComponent() = default;
		explicit SpotLightComponent(const glm::vec3& color) { spot.Color = color; };
	};

	struct ScriptComponent {
		std::filesystem::path filepath;

		ScriptComponent() = default;
		explicit ScriptComponent(std::filesystem::path filepath) : filepath(std::move(filepath)) {};
	};

	struct ColliderComponent {};
	struct AudioComponent {};
	struct AnimationComponent {};
	struct NetworkComponent {};

	template<typename... Component>
	struct ComponentGroup{};

	// every component should be included here
	using AllComponents = ComponentGroup<
			TransformComponent,
			MeshComponent,
			ScriptComponent,
			PointLightComponent,
			SpotLightComponent,
			EnvironmentComponent,
			DirectionalLightComponent
			>;
	// all singleton components, components that can only be on a single entity at a time
	using UniqueComponents = ComponentGroup<
	        EnvironmentComponent,
			DirectionalLightComponent
	        >;
}