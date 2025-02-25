//
// Created by Hayden Rivas on 1/21/25.
//
#include <regex>
#include <vector>

#include "Slate/Scene.h"
#include "Slate/Components.h"
#include "Slate/Entity.h"

namespace Slate {

	Entity Scene::CreateEntity() {
		return CreateEntity("Unnamed Entity");
	}
	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity = { _entityRegistry.create(), _entityRegistry };
		entity.AddComponent<CoreComponent>(name);
		return entity;
	}
	void Scene::DestroyEntity(const Entity& entity) {
		_entityRegistry.destroy(entity.GetHandle());
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src) {
		CopyComponentIfExists<Component...>(dst, src);
	}
	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src) {
		([&]() {
			if (src.HasComponent<Component>())
				dst.AddComponent<Component>(src.GetComponent<Component>());
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
	Entity Scene::DuplicateEntity(Entity entity) {
		const std::string name = entity.GetComponent<CoreComponent>().name;

		const std::string newName = GenerateUniqueName(name); // just 1++ to the back of the name
		Entity newEntity = CreateEntity(newName);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}







	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T &component) {
		static_assert(sizeof(T) == 0); // we want every templated component to have its own addition function
	}
	template<>
	void Scene::OnComponentAdded<CoreComponent>(Entity entity, CoreComponent &component) {

	}
	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component) {

	}
	template<>
	void Scene::OnComponentAdded(Slate::Entity entity, EnvironmentComponent& component) {

	}


}