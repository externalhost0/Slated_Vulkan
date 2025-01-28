//
// Created by Hayden Rivas on 1/21/25.
//

#pragma once
#include <vector>

#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>

#include "Components.h"

// templated functions are seperate to avoid cylic dependencies in their defintions, find them in "SceneTemplates.h"
namespace Slate {
	class Entity;

	class Scene {
	public:
		Scene() {
				_entityRegistry.on_construct<MeshComponent>().connect<&entt::registry::get_or_emplace<TransformComponent>>();
				_entityRegistry.on_destroy<TransformComponent>().connect<&entt::registry::remove<MeshComponent>>();
		};
		~Scene() = default;

		Entity CreateEntity();
		Entity CreateEntity(const std::string& name);

		Entity DuplicateEntity(Entity entity);

		void DestroyEntity(const Entity& entity);

	public:
		template<typename... Components>
		auto GetAllIDsWith();

		template<typename... Components>
		std::vector<Entity> GetAllEntitiesWith();
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry _entityRegistry;
		friend class Entity;
	};
}