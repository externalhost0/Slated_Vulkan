//
// Created by Hayden Rivas on 1/20/25.
//
#pragma once
#include <filesystem>
#include <string>
#include <utility>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "VK/vktypes.h"
#include "Slate/SmartPointers.h"
#include "Slate/Resources/ShaderResource.h"
#include "Slate/Resources/MeshResource.h"

namespace Slate {
	namespace Default {
		namespace Vec3 {
			static constexpr inline glm::vec3 Zero = { 0.f, 0.f, 0.f };
			static constexpr inline glm::vec3 One  = { 1.f, 1.f, 1.f };
		}
		namespace Quat {
			static constexpr inline glm::quat Zero = { 1.f, 0.f, 0.f, 0.f };
		}
	}

	enum class ComponentType : char {
		Transform,
		GeometryPrimitive,
		GeometryGLTF,
		Renderable,
		Audio,
		Script,

		PointLight,
		SpotLight,
		DirectionalLight,
		AmbientLight
	};
	static std::string GetComponentTypeName(ComponentType type) {
		static const std::unordered_map<ComponentType, const char*> map = {
				{ComponentType::Transform, "Transform"},
				{ComponentType::GeometryPrimitive, "Geometry Primitive"},
				{ComponentType::GeometryGLTF, "Geometry GLTF"},
				{ComponentType::Renderable, "Renderable"},
				{ComponentType::Audio, "Audio"},
				{ComponentType::Script, "Script"},

				{ComponentType::PointLight, "Point Light"},
				{ComponentType::SpotLight, "Spot Light"},
				{ComponentType::DirectionalLight, "Directional Light"},
				{ComponentType::AmbientLight, "Ambient Light"}
		};
		return map.at(type);
	}
	struct ComponentBase {
		const ComponentType type;
		constexpr explicit ComponentBase(ComponentType t) : type(t) {}
	};
	template <ComponentType T>
	struct Component : ComponentBase {
		constexpr Component() : ComponentBase(T) {}
	};


	struct TransformComponent : Component<ComponentType::Transform> {
		glm::vec3 position { 0.f };
		glm::quat rotation { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 scale    { 1.f };

		glm::vec3 local_position { 0.f };
		glm::quat local_rotation { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 local_scale    { 1.f };
	};

	enum class MeshPrimitiveType : char {
		Empty,
		// editor provided
		Quad,
		Plane,
		Cube,
		Sphere
	};
	struct GeometryPrimitiveComponent : Component<ComponentType::GeometryPrimitive> {
		MeshPrimitiveType mesh_type = MeshPrimitiveType::Empty;
	};
	struct GeometryGLTFComponent : Component<ComponentType::GeometryGLTF> {
		StrongPtr<MeshResource> mesh_source;
	};
	struct RenderableComponent : Component<ComponentType::Renderable> {
		StrongPtr<ShaderResource> shader_source;
		bool castShadows;
		bool recieveShadows;
	};

	enum class LightAppearance : char {
		COLOR,
		TEMPERATURE
	};
	struct ILightComponent {
		LightAppearance appearance = LightAppearance::COLOR;
	};
	struct PointLightComponent : ILightComponent, Component<ComponentType::PointLight> {
		GPU::PointLight point;
	};
	struct SpotLightComponent : ILightComponent, Component<ComponentType::SpotLight> {
		GPU::SpotLight spot;
	};
	struct DirectionalLightComponent : ILightComponent, Component<ComponentType::DirectionalLight> {
		GPU::DirectionalLight directional;
	};
	struct AmbientLightComponent : ILightComponent, Component<ComponentType::AmbientLight> {
		GPU::AmbientLight ambient;
	};

	static const std::unordered_map<MeshPrimitiveType, const char*> MeshPrimitiveTypeStringMap = {
			{MeshPrimitiveType::Empty, "Empty"},
			{MeshPrimitiveType::Quad, "Quad"},
			{MeshPrimitiveType::Plane, "Plane"},
			{MeshPrimitiveType::Cube, "Cube"},
			{MeshPrimitiveType::Sphere, "Sphere"}
	};

	struct ScriptComponent : Component<ComponentType::Script> {

	};
	struct AudioComponent : Component<ComponentType::Audio> {
		std::string name;
	};

	struct ColliderComponent {};
	struct AnimationComponent {};
	struct NetworkComponent {};

	template<typename... T>
	struct ComponentGroup{};

	// every component should be included here
	using AllComponents = ComponentGroup<
			TransformComponent,
			GeometryPrimitiveComponent,
			GeometryGLTFComponent,
			ScriptComponent,
			AudioComponent,

			PointLightComponent,
			SpotLightComponent,
			DirectionalLightComponent,
			AmbientLightComponent
			>;
	// all singleton components, components that can only be on a single entity at a time
	using UniqueComponents = ComponentGroup<
			AmbientLightComponent,
			DirectionalLightComponent
	        >;
}