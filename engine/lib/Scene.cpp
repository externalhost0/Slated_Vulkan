//
// Created by Hayden Rivas on 1/21/25.
//
#include <regex>
#include <vector>

#include "Slate/Scene.h"
#include "Slate/Components.h"
#include "Slate/Entity.h"
#include "Slate/Logger.h"

namespace Slate {
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
		register_group_dependencies<TransformComponent, TransformDependants>(registry);
	}
	void Scene::Clear() {
		this->registry.clear<entt::entity>();
		this->entityMap.clear();
	}

	Entity& Scene::CreateEntity() {
		return CreateEntity("Unnamed Entity");
	}
	Entity& Scene::CreateEntity(const std::string& name) {
		const entt::entity handle = this->registry.create();
		auto entityptr = CreateUniquePtr<Entity>(handle, this->shared_from_this());
		entityptr->SetName(name);
		this->entityMap[handle] = std::move(entityptr);
		return *this->entityMap[handle];
	}
	// recurse through children when destroying
	void Scene::DestroyEntity(const entt::entity& handle) {
		auto& trash_entity = this->GetEntity(handle);
		if (auto parentPtr = trash_entity.GetParentPtr()) {
			parentPtr->RemoveChild(handle);
		}
		auto children = trash_entity.GetChildren();
		for (auto child : children) {
			DestroyEntity(child);
		}
		this->entityMap.erase(handle);
		this->registry.destroy(handle);
	}

	template<typename... T>
	static void CopyComponentIfExists(ComponentGroup<T...>, Entity& dst, Entity& src) {
		CopyComponentIfExists<T...>(dst, src);
	}
	template<typename... T>
	static void CopyComponentIfExists(Entity& dst, Entity& src) {
		([&]() {
			if (src.HasComponent<T>())
				dst.AddComponent<T>(src.GetComponent<T>());
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
	Entity& Scene::DuplicateEntity(const entt::entity& handle) {
		const std::string name = entityMap[handle]->GetName();
		const std::string new_name = GenerateUniqueName(name); // just 1++ to the back of the name
		auto& new_entity = CreateEntity(new_name);
		CopyComponentIfExists(AllComponents{}, GetEntity(new_entity.GetHandle()), GetEntity(handle));
		return new_entity;
	}
	Entity& Scene::GetEntity(const entt::entity& handle) {
		auto it = entityMap.find(handle);
		if (it != entityMap.end()) {
			return *it->second;
		}
		throw RUNTIME_ERROR("Attempt to get entity ({}), does not exist!", (uint32_t)handle);
	}

	std::vector<Entity*> Scene::GetAllEntities() {
		std::vector<Entity*> entities_to_grab;
		for (const entt::entity& handle : this->registry.view<entt::entity>()) {
			entities_to_grab.emplace_back(this->entityMap[handle].get());
		}
		return entities_to_grab;
	}
	std::vector<Entity*> Scene::GetTopLevelEntities() {
		std::vector<Entity*> entities_to_grab;
		for (const entt::entity& handle : this->registry.view<entt::entity>()) {
			const auto& entity = this->entityMap[handle];
			if (entity->HasParent()) {
				continue;
			}
			entities_to_grab.emplace_back(entity.get());
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
	void Scene::OnComponentAdded<GeometryPrimitiveComponent>(Entity entity, GeometryPrimitiveComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<GeometryGLTFComponent>(Entity entity, GeometryGLTFComponent& component) {

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
	void Scene::OnComponentAdded<AmbientLightComponent>(Entity entity, AmbientLightComponent& component) {

	}


}