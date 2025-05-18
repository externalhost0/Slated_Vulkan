//
// Created by Hayden Rivas on 1/20/25.
//
#pragma once
#include <filesystem>
#include <string>
#include <utility>
#include <entt/entity/entity.hpp>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Slate/Resources/MeshResource.h"
#include "Slate/Resources/ScriptResource.h"
#include "Slate/Resources/ShaderResource.h"
#include "Slate/SmartPointers.h"
#include "Slate/VK/vktypes.h"

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

	// these two need to be 1 : 1 in order
	enum class ComponentType : uint8_t {
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
	constexpr const char* component_type_names[] = {
			"Transform",
			"Geometry Primitive",
			"Geometry GLTF",
			"Renderable",
			"Audio",
			"Script",

			"Point Light",
			"Spot Light",
			"Directional Light",
			"Ambient Light"
	};
	constexpr const char* GetComponentTypeName(ComponentType type) {
		return component_type_names[(int) type];
	}
	struct ComponentBase {
		const ComponentType type;
		constexpr explicit ComponentBase(ComponentType t) : type(t) {}
	};
	template <ComponentType T>
	struct IComponent : ComponentBase {
		constexpr IComponent() : ComponentBase(T) {}
	};

	enum class Tag {
		None = 0,
		Camera,
		Water
	};
	static constexpr unsigned int MAX_CHILD_COUNT = 128;


	struct Transform {
		glm::vec3 position { 0.f };
		glm::quat rotation { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 scale    { 1.f };
	};
	struct TransformComponent : IComponent<ComponentType::Transform> {
		Transform global;
		Transform local;
	};

	enum class MeshPrimitiveType : char {
		Empty = 0,
		// editor provided
		Quad,
		Plane,
		Cube,
		Sphere
	};
	struct GeometryPrimitiveComponent : IComponent<ComponentType::GeometryPrimitive> {
		MeshPrimitiveType mesh_type = MeshPrimitiveType::Empty;
	};
	struct GeometryGLTFComponent : IComponent<ComponentType::GeometryGLTF> {
		StrongPtr<MeshResource> mesh_source;
	};
	struct RenderableComponent : IComponent<ComponentType::Renderable> {
		StrongPtr<ShaderResource> shader_source;
		uint16_t _gen;
		bool castShadows = true;
		bool recieveShadows = true;
	};

	enum class LightAppearance : char {
		COLOR,
		TEMPERATURE
	};
	struct ILightComponent {
		LightAppearance appearance = LightAppearance::COLOR;
	};
	struct PointLightComponent : ILightComponent, IComponent<ComponentType::PointLight> {
		GPU::PointLight point;
	};
	struct SpotLightComponent : ILightComponent, IComponent<ComponentType::SpotLight> {
		GPU::SpotLight spot;
	};
	struct DirectionalLightComponent : ILightComponent, IComponent<ComponentType::DirectionalLight> {
		GPU::DirectionalLight directional;
	};
	struct AmbientLightComponent : ILightComponent, IComponent<ComponentType::AmbientLight> {
		GPU::AmbientLight ambient;
	};


	static const std::vector<std::pair<MeshPrimitiveType, const char*>> MeshPrimitiveTypeStringMap = {
			{MeshPrimitiveType::Empty, "Empty"},
			{MeshPrimitiveType::Quad, "Quad"},
			{MeshPrimitiveType::Plane, "Plane"},
			{MeshPrimitiveType::Cube, "Cube"},
			{MeshPrimitiveType::Sphere, "Sphere"}
	};

	struct ScriptComponent : IComponent<ComponentType::Script> {
		StrongPtr<ScriptResource> script_source;
	};
	struct AudioComponent : IComponent<ComponentType::Audio> {
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