//
// Created by Hayden Rivas on 1/21/25.
//
#include <regex>
#include <vector>

#include "Slate/Scene.h"
#include "Slate/Components.h"
#include "Slate/Entity.h"

namespace Slate {

	// TODO: make helper function or map for this sorta thing
	Scene::Scene() {
		this->registry.on_construct<MeshPrimitiveComponent>().connect<&entt::registry::get_or_emplace<TransformComponent>>();
		this->registry.on_destroy<TransformComponent>().connect<&entt::registry::remove<MeshPrimitiveComponent>>();

		this->registry.on_construct<MeshGLTFComponent>().connect<&entt::registry::get_or_emplace<TransformComponent>>();
		this->registry.on_destroy<TransformComponent>().connect<&entt::registry::remove<MeshGLTFComponent>>();

		this->registry.on_construct<PointLightComponent>().connect<&entt::registry::get_or_emplace<TransformComponent>>();
		this->registry.on_destroy<TransformComponent>().connect<&entt::registry::remove<PointLightComponent>>();

		this->registry.on_construct<DirectionalLightComponent>().connect<&entt::registry::get_or_emplace<TransformComponent>>();
		this->registry.on_destroy<TransformComponent>().connect<&entt::registry::remove<DirectionalLightComponent>>();

		this->registry.on_construct<SpotLightComponent>().connect<&entt::registry::get_or_emplace<TransformComponent>>();
		this->registry.on_destroy<TransformComponent>().connect<&entt::registry::remove<SpotLightComponent>>();
	}
	void Scene::Clear() {
		this->registry.clear<entt::entity>();
//		this->entityMap.clear();
	}

	Shared<Entity> Scene::CreateEntity() {
		return CreateEntity("Unnamed Entity");
	}
	Shared<Entity> Scene::CreateEntity(const std::string& name) {
		entt::entity handle = this->registry.create();
		Shared<Entity> entity = CreateShared<Entity>(handle, this->shared_from_this());
		entity->name = name;
		this->entityMap[handle] = entity;
		return entity;
	}
	void Scene::DestroyEntityById(const entt::entity& id) {
		this->entityMap.erase(id);
		this->registry.destroy(id);
	}
	void Scene::DestroyEntity(const Shared<Entity>& entity) {
		auto handle = entity->GetHandle();
		this->entityMap.erase(handle);
		this->registry.destroy(handle);
	}

	template<typename... T>
	static void CopyComponentIfExists(ComponentGroup<T...>, Shared<Entity> dst, Shared<Entity> src) {
		CopyComponentIfExists<T...>(dst, src);
	}
	template<typename... T>
	static void CopyComponentIfExists(Shared<Entity> dst, Shared<Entity> src) {
		([&]() {
			if (src->HasComponent<T>())
				dst->AddComponent<T>(src->GetImmutableComponent<T>());
		}(), ...);
	}


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
	Shared<Entity> Scene::DuplicateEntity(const Shared<Entity>& entity) {
		const std::string name = entity->GetName();
		const std::string new_name = GenerateUniqueName(name); // just 1++ to the back of the name
		Shared<Entity> new_entity = CreateEntity(new_name);
		CopyComponentIfExists(AllComponents{}, new_entity, entity);
		return new_entity;
	}
	Shared<Entity> Scene::GetEntityById(const entt::entity& id) {
		return this->entityMap[id];
	}

	std::vector<Shared<Entity>> Scene::GetAllEntities() {
		std::vector<Shared<Entity>> entities_to_grab;
		for (const entt::entity& handle : this->registry.view<entt::entity>()) {
			entities_to_grab.emplace_back(this->entityMap[handle]);
		}
		return entities_to_grab;
	}
	std::vector<Shared<Entity>> Scene::GetTopLevelEntities() {
		std::vector<Shared<Entity>> entities_to_grab;
		for (const entt::entity& handle : this->registry.view<entt::entity>()) {
			Shared<Entity> entity = this->entityMap[handle];
			if (entity->HasParent()) {
				continue;
			}
			entities_to_grab.emplace_back(entity);
		}
		return entities_to_grab;
	}



	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T &component) {
		static_assert(sizeof(T) == 0); // we want every templated component to have its own addition function
	}
	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<MeshPrimitiveComponent>(Entity entity, MeshPrimitiveComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<MeshGLTFComponent>(Entity entity, MeshGLTFComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<SpotLightComponent>(Entity entity, SpotLightComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<EnvironmentComponent>(Entity entity, EnvironmentComponent& component) {

	}


}