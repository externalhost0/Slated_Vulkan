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

				_entityRegistry.on_construct<PointLightComponent>().connect<&entt::registry::get_or_emplace<TransformComponent>>();
				_entityRegistry.on_destroy<TransformComponent>().connect<&entt::registry::remove<PointLightComponent>>();

				_entityRegistry.on_construct<SpotLightComponent>().connect<&entt::registry::get_or_emplace<TransformComponent>>();
				_entityRegistry.on_destroy<TransformComponent>().connect<&entt::registry::remove<SpotLightComponent>>();
		};
		~Scene() = default;

		Entity CreateEntity();
		Entity CreateEntity(const std::string& name);

		Entity DuplicateEntity(Entity entity);

		void DestroyEntity(const Entity& entity);

		entt::registry& GetRegistry() { return _entityRegistry; };
	public:
		template<typename... Components>
		auto GetAllIDsWith();

		template<typename... Components>
		std::vector<Entity> GetAllEntitiesWith();


		EnvironmentComponent& GetEnvironment() {
			auto view = _entityRegistry.view<EnvironmentComponent>();
			if (view->empty()) {
				return this->_environment;
			} else {
				entt::entity temp = *_entityRegistry.view<EnvironmentComponent>().begin();
				return _entityRegistry.get<EnvironmentComponent>(temp);
			}
		}
		DirectionalLightComponent& GetDirectional() {
			auto view = _entityRegistry.view<DirectionalLightComponent>();
			if (view->empty()) {
				return this->_directional;
			} else {
				entt::entity temp = *_entityRegistry.view<DirectionalLightComponent>().begin();
				return _entityRegistry.get<DirectionalLightComponent>(temp);
			}
		}



	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		DirectionalLightComponent _directional {};
		EnvironmentComponent _environment {}; // use all default values of environment
		entt::registry _entityRegistry;
		friend class Entity;
	};
}