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
		Entity entity = { _entityRegistry.create(), this };
		entity.AddComponent<CoreComponent>(name);
		return entity;
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
			std::string namePart = match[1].str();
			std::string numberPart = match[3].str();

			int number = numberPart.empty() ? 1 : std::stoi(numberPart) + 1;
			return namePart + " " + std::to_string(number);
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

	void Scene::DestroyEntity(const Entity& entity) {
		_entityRegistry.destroy(entity.GetHandle());
	}





	template<typename T>
	void Scene::OnComponentAdded(Slate::Entity entity, T &component) {
		static_assert(sizeof(T) == 0); // we want every templated component to have its own addition function
	}
	template<>
	void Scene::OnComponentAdded<CoreComponent>(Slate::Entity entity, CoreComponent &component) {

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


}