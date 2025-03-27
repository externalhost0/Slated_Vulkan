//
// Created by Hayden Rivas on 1/21/25.
//

#pragma once
#include <vector>
#include <unordered_map>

#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>

#include "Components.h"
#include "SmartPointers.h"

// templated functions are seperate to avoid cylic dependencies in their defintions, find them in "SceneTemplates.h"
namespace Slate {
	class Entity;

	class Scene : public std::enable_shared_from_this<Scene> {
	public:
		Scene();
		~Scene() = default;

		Shared<Entity> CreateEntity();
		Shared<Entity> CreateEntity(const std::string& name);

		Shared<Entity> DuplicateEntity(const Shared<Entity>& entity);

		void DestroyEntity(const Shared<Entity>& entity);
		void DestroyEntityById(const entt::entity& id);

		Shared<Entity> GetEntityById(const entt::entity& id);
		std::vector<Shared<Entity>> GetTopLevelEntities();
		std::vector<Shared<Entity>> GetAllEntities();
		template<class... T>
		std::vector<Shared<Entity>> GetAllEntitiesWith();

		entt::registry& GetRegistry() { return registry; };

		void Clear();
	public:
		EnvironmentComponent& GetEnvironment() {
			auto view = registry.view<EnvironmentComponent>();
			if (view->empty()) {
				return this->_environment;
			} else {
				entt::entity temp = *registry.view<EnvironmentComponent>().begin();
				return registry.get<EnvironmentComponent>(temp);
			}
		}
		DirectionalLightComponent& GetDirectional() {
			auto view = registry.view<DirectionalLightComponent>();
			if (view->empty()) {
				return this->_directional;
			} else {
				entt::entity temp = *registry.view<DirectionalLightComponent>().begin();
				return registry.get<DirectionalLightComponent>(temp);
			}
		}

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:

		DirectionalLightComponent _directional {};
		EnvironmentComponent _environment {}; // use all default values of environment

		entt::registry registry;
		std::unordered_map<entt::entity, Shared<Entity>> entityMap;
	};
}