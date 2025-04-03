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
	static constexpr uint8_t MAX_ENTITY_COUNT = 255;

	class Scene : public std::enable_shared_from_this<Scene> {
	public:
		Scene();
		~Scene() = default;

		Entity& CreateEntity();
		Entity& CreateEntity(const std::string& name);

		Entity& DuplicateEntity(const entt::entity& handle);

		void DestroyEntity(const entt::entity& handle);

		Entity& GetEntity(const entt::entity& handle);
		std::vector<Entity*> GetTopLevelEntities();
		std::vector<Entity*> GetAllEntities();
		template<class... T>
		std::vector<Entity*> GetAllEntitiesWith();

		entt::registry& GetRegistry() { return registry; };
		unsigned int GetEntityCount() { return entityMap.size(); }

		void Clear();
	public:
		AmbientLightComponent& GetEnvironment() {
			auto view = registry.view<AmbientLightComponent>();
			if (view->empty()) {
				return this->_environment;
			} else {
				entt::entity temp = *registry.view<AmbientLightComponent>().begin();
				return registry.get<AmbientLightComponent>(temp);
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
		AmbientLightComponent _environment {}; // use all default values of environment

		entt::registry registry;
		std::unordered_map<entt::entity, UniquePtr<Entity>> entityMap;
	};
}