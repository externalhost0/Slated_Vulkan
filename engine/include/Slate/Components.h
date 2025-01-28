//
// Created by Hayden Rivas on 1/20/25.
//
#pragma once
#include <filesystem>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <utility>

#include "Slate/VK/vktypes.h"

namespace Slate {
	struct CoreComponent {
		std::string name {"Unnamed Entity"};
	};

	struct TransformComponent {
		glm::vec3 Position {0.f};
		glm::quat Rotation {1.f, 0.f, 0.f, 0.f};
		glm::vec3 Scale    {1.f};

		TransformComponent() = default;
		explicit TransformComponent(glm::vec3 pos) : Position(pos) {};
	};


	struct MeshComponent {
		vktypes::MeshData data;

		MeshComponent() = default;
		explicit MeshComponent(vktypes::MeshData data) : data(data) {}
	};

	struct ScriptComponent {
		std::filesystem::path filepath;

		ScriptComponent() = default;
		explicit ScriptComponent(std::filesystem::path filepath) : filepath(std::move(filepath)) {};
	};
	struct ColliderComponent {};
	struct AudioComponent {};
	struct LightComponent {};
	struct AnimationComponent {};
	struct NetworkComponent {};

	template<typename... Component>
	struct ComponentGroup{};

	using AllComponents = ComponentGroup<
					TransformComponent,
					MeshComponent,
					ScriptComponent
					>;
}