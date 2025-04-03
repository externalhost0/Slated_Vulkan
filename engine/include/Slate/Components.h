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
		EnvironmentLight
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
				{ComponentType::EnvironmentLight, "Environment Light"}
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
		glm::vec3 Position { 0.f };
		glm::quat Rotation { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 Scale    { 1.f };
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
		MeshPrimitiveType Meshtype = MeshPrimitiveType::Empty;
	};
	struct GeometryGLTFComponent : Component<ComponentType::GeometryGLTF> {
		StrongPtr<MeshResource> mesh;
	};
	struct RenderableComponent : Component<ComponentType::Renderable> {
		StrongPtr<ShaderResource> shader;
		bool castShadows;
		bool recieveShadows;
	};

	enum class LightAppearance : char {
		COLOR,
		TEMPERATURE
	};
	struct ILightComponent {
		LightAppearance Appearance = LightAppearance::COLOR;
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
	struct AmbientLightComponent : ILightComponent, Component<ComponentType::EnvironmentLight> {
		GPU::AmbientLight ambient;
	};

	static const std::unordered_map<MeshPrimitiveType, const char*> MeshPrimitiveTypeStringMap = {
			{MeshPrimitiveType::Quad, "Quad"},
			{MeshPrimitiveType::Plane, "Plane"},
			{MeshPrimitiveType::Cube, "Cube"},
			{MeshPrimitiveType::Sphere, "Sphere"},
			{MeshPrimitiveType::Empty, "Empty"}
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