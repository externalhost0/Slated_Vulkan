//
// Created by Hayden Rivas on 1/21/25.
//

#pragma once
#include <vector>
#include <unordered_map>

#include <entt/entity/registry.hpp>
#include <entt/entity/view.hpp>

#include "Components.h"
#include "Slate/SmartPointers.h"
#include "Slate/SystemManager.h"

// templated functions are seperate to avoid cylic dependencies in their defintions, find them in "SceneTemplates.h"
namespace Slate {
	class GameEntity;
	static constexpr uint8_t MAX_ENTITY_COUNT = 255;

	enum class SceneState : uint8_t {
		Dead = 0,
		Loading,
		Active,
		Destroyed
	};

	class Scene final {
	public:
		Scene();
		~Scene();

		// entity interaction
		GameEntity CreateEntity();
		GameEntity CreateEntity(const std::string& name);
		GameEntity DuplicateEntity(GameEntity entity);
		void DestroyEntity(GameEntity entity);
		void DestroyEntity(entt::entity handle);
		GameEntity GetEntityByName(const char* name);

		// entity retrieval
		std::vector<GameEntity> GetRootEntities();
		std::vector<GameEntity> GetAllEntities();
		template<class... T>
		std::vector<GameEntity> GetAllEntitiesWithEXT();
	public:
		void Tick(double deltaTime);
		void Render();

		// try not call resolve
		GameEntity resolveEntity(entt::entity handle);
	public:
		AmbientLightComponent& GetEnvironment() {
			auto view = _registry.view<AmbientLightComponent>();
			if (view->empty()) {
				return this->_environment;
			} else {
				entt::entity temp = *_registry.view<AmbientLightComponent>().begin();
				return _registry.get<AmbientLightComponent>(temp);
			}
		}
		DirectionalLightComponent& GetDirectional() {
			auto view = _registry.view<DirectionalLightComponent>();
			if (view->empty()) {
				return this->_directional;
			} else {
				entt::entity temp = *_registry.view<DirectionalLightComponent>().begin();
				return _registry.get<DirectionalLightComponent>(temp);
			}
		}
		template<typename... T, typename Func>
		void forEach(Func&& callback) {
			auto view = _registry.view<T...>();
			for (entt::entity entity : view) {
				callback(view.template get<T>(entity)...);
			}
		}
		template<typename... T, typename Func>
		void forEachRoot(Func&& callback) {
			auto view = _registry.view<CoreComponent, T...>();
			for (entt::entity entity : view) {
				const CoreComponent& core = view.template get<CoreComponent>(entity);
				if (core.parent == entt::null) {
					callback(view.template get<T>(entity)...);
				}
			}
		}
	private:
		DirectionalLightComponent _directional {};
		AmbientLightComponent _environment {}; // use all default values of environment

		SystemManager systemManager;
		entt::registry _registry;
	};
}

