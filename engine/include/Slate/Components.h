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

namespace Slate {
	enum class ComponentType {
		Transform,
		MeshGLTF,
		MeshPrimitive,
		Audio,
		Script,

		PointLight,
		SpotLight,
		DirectionalLight,
		EnvironmentLight
	};
	static const std::unordered_map<ComponentType, const char*> ComponentTypeStringMap = {
			{ComponentType::Transform, "Transform"},
			{ComponentType::MeshGLTF, "Mesh GLTF"},
			{ComponentType::MeshPrimitive, "Mesh Primitive"},
			{ComponentType::Audio, "Audio"},
			{ComponentType::Script, "Script"},

			{ComponentType::PointLight, "Point Light"},
			{ComponentType::SpotLight, "Spot Light"},
			{ComponentType::DirectionalLight, "Directional Light"},
			{ComponentType::EnvironmentLight, "Environment Light"}
	};
	struct IComponentBase {
	public:
		IComponentBase() = delete;
	protected:
		constexpr explicit IComponentBase(ComponentType type) : type(type) {}
	public:
		ComponentType GetComponentType() const { return type; }
		std::string GetComponentTypeName() const { return ComponentTypeStringMap.at(type); }
	private:
		const ComponentType type;
	};
	template <ComponentType T>
	struct IComponent : IComponentBase {
	protected: constexpr IComponent() : IComponentBase(T) {}
	};


	struct TransformComponent : IComponent<ComponentType::Transform> {
		glm::vec3 Position { 0.f };
		glm::quat Rotation { 1.f, 0.f, 0.f, 0.f };
		glm::vec3 Scale    { 1.f };

		TransformComponent() = default;
		explicit TransformComponent(glm::vec3 pos) : Position(pos) {};
	};

	enum class MeshPrimitiveType : uint8_t {
		// editor provided
		Quad,
		Plane,
		Cube,
		Sphere,
		None,
	};
	static const std::unordered_map<MeshPrimitiveType, const char*> MeshPrimitiveTypeStringMap = {
			{MeshPrimitiveType::Quad, "Quad"},
			{MeshPrimitiveType::Plane, "Plane"},
			{MeshPrimitiveType::Cube, "Cube"},
			{MeshPrimitiveType::Sphere, "Sphere"},

			{MeshPrimitiveType::None, "None"}
	};

	// either an editor primitive or a assets single shape
	class MeshBuffer {
	public:
		MeshBuffer() = default;
		~MeshBuffer() = default;
		
		bool IsIndexed() const { return indexCount > 0; }
		const vktypes::AllocatedBuffer& GetIndexBuffer() const { return this->indexBuffer; }
		const VkDeviceAddress& GetVBA() const { return this->vertexBufferAddress; }
		uint32_t GetVertexCount() const { return this->vertexCount; }
		uint32_t GetIndexCount() const { return this->indexCount; }
	private:
		vktypes::AllocatedBuffer indexBuffer;
		VkDeviceAddress vertexBufferAddress = 0;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;

		friend class RenderEngine;
		friend class Editor;
	};

	class MeshAsset {
	public:
		MeshAsset() = default;
		~MeshAsset() = default;

		uint32_t GetVertexCount() const {
			uint32_t vertex_count = 0;
			for (const auto& buffer : this->buffers) {
				vertex_count += buffer->GetVertexCount();
			}
			return vertex_count;
		};
		uint32_t GetIndexCount() const {
			uint32_t index_count = 0;
			for (const auto& buffer : this->buffers) {
				index_count += buffer->GetIndexCount();
			}
			return index_count;
		};
		std::string GetAssetName() const {
			if (this->metadata.has_value()) {
				return this->metadata->name;
			}
			return "Null";
		}
		void SetAssetName(const std::string& new_name) {
			this->metadata->name = new_name;
		}
		std::vector<Shared<MeshBuffer>> GetMeshBuffers() const {
			return this->buffers;
		}
	private:
		struct AssetMetadata {
			std::string name;
			std::filesystem::path filepath;
		};
		Optional<AssetMetadata> metadata = std::nullopt;
		std::vector<Shared<MeshBuffer>> buffers;
	};

	struct ShaderAsset {
	public:
		ShaderAsset() = default;

		VkPipeline GetPipeline() const { return this->pipeline; }
		VkPipelineLayout GetPipelineLayout() const { return this->layout; }
	private:
		std::string name;
		std::filesystem::path filepath;

		VkPipeline pipeline;
		VkPipelineLayout layout;
	};




	struct MeshPrimitiveComponent : IComponent<ComponentType::MeshPrimitive> {
	public:
		MeshPrimitiveComponent() = default;
		MeshPrimitiveComponent(const MeshBuffer& buffer, MeshPrimitiveType type) {
			this->buffer = CreateShared<MeshBuffer>(buffer);
			this->type = type;
		}
		Shared<MeshBuffer> GetMeshBuffer() const { return this->buffer; }
	public:
		MeshPrimitiveType GetMeshType() const { return this->type; }
		std::string GetMeshTypeName() const { return MeshPrimitiveTypeStringMap.at(this->type); }
		void ChangeMeshType(const MeshPrimitiveType& new_meshtype) { this->type = new_meshtype; }
	private:
		Shared<MeshBuffer> buffer;
		MeshPrimitiveType type = MeshPrimitiveType::None;
	};

	struct MeshGLTFComponent : IComponent<ComponentType::MeshGLTF> {
	public:
		MeshGLTFComponent() = default;
		explicit MeshGLTFComponent(const MeshAsset& asset) {
			this->asset = CreateShared<MeshAsset>(asset);
		}
		Shared<MeshAsset> GetMesh() const { return this->asset; }
	private:
		Shared<MeshAsset> asset;
	};


	enum class LightAppearance {
		COLOR,
		TEMPERATURE
	};
	struct EnvironmentComponent : IComponent<ComponentType::EnvironmentLight> {
		GPU::AmbientLight ambient;

		EnvironmentComponent() = default;
	};
	struct DirectionalLightComponent : IComponent<ComponentType::DirectionalLight> {
		GPU::DirectionalLight directional;
		LightAppearance appearance = LightAppearance::COLOR;

		DirectionalLightComponent() = default;
		explicit DirectionalLightComponent(const glm::vec3& color) { directional.Color = color; };
	};
	struct PointLightComponent : IComponent<ComponentType::PointLight> {
		GPU::PointLight point;
		LightAppearance appearance = LightAppearance::COLOR;

		PointLightComponent() = default;
		explicit PointLightComponent(const glm::vec3& color) { point.Color = color; };
	};
	struct SpotLightComponent : IComponent<ComponentType::SpotLight> {
		GPU::SpotLight spot;
		LightAppearance appearance = LightAppearance::COLOR;

		SpotLightComponent() = default;
		explicit SpotLightComponent(const glm::vec3& color) { spot.Color = color; };
	};

	struct ScriptComponent : IComponent<ComponentType::Script> {
		std::filesystem::path filepath;

		ScriptComponent() = default;
		explicit ScriptComponent(std::filesystem::path filepath) : filepath(std::move(filepath)) {};
	};

	struct ColliderComponent {};
	struct AudioComponent {};
	struct AnimationComponent {};
	struct NetworkComponent {};

	template<typename... T>
	struct ComponentGroup{};

	// every component should be included here
	using AllComponents = ComponentGroup<
			TransformComponent,
			ScriptComponent,
			MeshPrimitiveComponent,
			MeshGLTFComponent,

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