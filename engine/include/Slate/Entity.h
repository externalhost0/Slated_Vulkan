//
// Created by Hayden Rivas on 1/21/25.
//
#pragma once
#include <entt/entity/entity.hpp>

#include "Debug.h"
#include "Scene.h"

namespace Slate {
	class Entity {
	public:
		Entity& operator=(const Entity&) = delete;
		~Entity() = default;
		Entity(entt::entity handle, entt::registry& registry) : _entityHandle(handle), _registry(registry) {};

		bool operator==(const Entity& other) const { return _entityHandle == other._entityHandle; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
		explicit operator bool() const { return _entityHandle != entt::null; }

		entt::entity GetHandle() const { return _entityHandle; };
	public:
		template<class T, typename... Args>
		T& AddComponent(Args&&... args) {
			if (HasComponent<T>()) throw std::runtime_error("Entity already has that component!");
			if (AllowedToHave<T>()) throw std::runtime_error("Entity is not allowed to have that unique component!");
			T& component = _registry.emplace<T>(_entityHandle, std::forward<Args>(args)...);
//			_pScene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent() {
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			return _registry.get<T>(_entityHandle);
		}

		template<typename T>
		bool HasComponent() {
			return _registry.all_of<T>(_entityHandle);
		}
		template<typename T>
		bool AllowedToHave() {
			return _registry.storage<EnvironmentComponent>().size() > 1;
		}

		template<typename T>
		void RemoveComponent() {
			EXPECT(HasComponent<T>(), "Entity does not have that component!");
			_registry.remove<T>(_entityHandle);
		}

	private:
		entt::entity _entityHandle = entt::null;
		entt::registry& _registry;

		std::vector<Entity*> _children;
	};
}