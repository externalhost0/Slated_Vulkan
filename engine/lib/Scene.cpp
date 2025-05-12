//
// Created by Hayden Rivas on 1/21/25.
//
#include <regex>
#include <vector>

#include "Slate/Common/Logger.h"
#include "Slate/ECS/Components.h"
#include "Slate/ECS/Entity.h"
#include "Slate/ECS/Scene.h"

#include "Slate/Systems/RenderSystem.h"
#include "Slate/Systems/TransformSystem.h"
#include "Slate/Systems/ShaderSystem.h"

namespace Slate {
	std::string GenerateUniqueName(const std::string& baseName) {
		std::regex regex("^(.*?)(\\s(\\d+))?$");
		std::smatch match;

		if (std::regex_match(baseName, match, regex)) {
			std::string name_segment = match[1].str();
			std::string number_segment = match[3].str();

			int newnumber = number_segment.empty() ? 1 : std::stoi(number_segment) + 1;
			return name_segment + " " + std::to_string(newnumber);
		}
		return baseName + " 2";
	}
	template<typename... T>
	static void CopyComponentIfExists(ComponentGroup<T...>, GameEntity dst, GameEntity src) {
		CopyComponentIfExists<T...>(dst, src);
	}
	template<typename... T>
	static void CopyComponentIfExists(GameEntity dst, GameEntity src) {
		([&]() {
			if (src.HasComponent<T>())
				dst.AddComponent<T>(src.GetComponent<T>());
		}(), ...);
	}


	// singular
	template <typename Dependency, typename Dependent>
	void register_dependency(entt::registry& registry) {
		registry.on_construct<Dependent>().template connect<&entt::registry::get_or_emplace<Dependency>>();
		registry.on_destroy<Dependency>().template connect<&entt::registry::remove<Dependent>>();
	}
	template <typename Dependency, typename Group>
	void register_group_dependencies(entt::registry& registry) {
		register_group_dependencies_impl<Dependency>(registry, Group{});
	}
	// unpacking of group
	template <typename Dependency, typename... Dependents>
	void register_group_dependencies_impl(entt::registry& registry, ComponentGroup<Dependents...>) {
		(register_dependency<Dependency, Dependents>(registry), ...);
	}

	using TransformDependants = ComponentGroup<
			GeometryGLTFComponent,
			GeometryPrimitiveComponent,
			PointLightComponent,
			SpotLightComponent,
			DirectionalLightComponent
			>;
	Scene::Scene() {
		register_group_dependencies<TransformComponent, TransformDependants>(_registry);

		systemManager.registerSystem<TransformSystem>();
		systemManager.registerSystem<RenderSystem>();
		systemManager.registerSystem<ShaderSystem>();

		systemManager.startAll(*this);
	}
	void Scene::Tick(double deltaTime) {
		systemManager.updateAll(*this);
	}
	void Scene::Render() {

	}
	Scene::~Scene() {
		systemManager.stopAll(*this);
		this->_registry.clear<entt::entity>();
	}

	GameEntity Scene::CreateEntity() {
		return CreateEntity("Unnamed Entity");
	}
	GameEntity Scene::CreateEntity(const std::string& name) {
		const entt::entity handle = this->_registry.create();
		_registry.emplace<CoreComponent>(handle, CoreComponent{.name = name});
		return { handle, this->_registry };
	}
	GameEntity Scene::DuplicateEntity(GameEntity entity) {
		const std::string& name = entity.GetName();
		const std::string& new_name = GenerateUniqueName(name); // just 1++ to the back of the name
		GameEntity new_entity = CreateEntity(new_name);
		CopyComponentIfExists(AllComponents{}, new_entity, entity);
		return new_entity;
	}
	void Scene::DestroyEntity(entt::entity handle) {
		DestroyEntity({handle, this->_registry});
	}
	// recurse through children when destroying
	void Scene::DestroyEntity(GameEntity entity) {
		if (entity.HasParent()) {
			entity.GetParent().RemoveChild(entity);
		}
		std::vector<GameEntity> children = entity.GetChildren();
		for (auto child : children) {
			DestroyEntity(child);
		}
		this->_registry.destroy(entity.GetHandle());
	}

	std::vector<GameEntity> Scene::GetAllEntities() {
		std::vector<GameEntity> result;
		for (entt::entity handle : this->_registry.view<entt::entity>()) {
			result.emplace_back(handle, this->_registry);
		}
		return result;
	}
	std::vector<GameEntity> Scene::GetRootEntities() {
		std::vector<GameEntity> result;
		for (const entt::entity handle : this->_registry.view<entt::entity>()) {
			GameEntity entity = GameEntity(handle, this->_registry);
			if (entity.HasParent()) {
				continue;
			}
			result.push_back(entity);
		}
		return result;
	}
	// dont really use this
	GameEntity Scene::resolveEntity(entt::entity handle) {
		return { handle, this->_registry };
	}
	GameEntity Scene::GetEntityByName(const char* name) {
		for (const entt::entity handle : this->_registry.view<entt::entity>()) {
			GameEntity entity = GameEntity(handle, this->_registry);
			if (entity.GetName() == name) {
				return entity;
			}
		}
		assert(false);
	}
}