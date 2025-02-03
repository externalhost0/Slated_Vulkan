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
		Entity() = default;
		Entity(const Entity& other) = default;
		Entity(entt::entity handle, Scene* scene) : _entityHandle(handle), _pScene(scene) {};

		bool operator==(const Entity& other) const { return _entityHandle == other._entityHandle; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
		explicit operator bool() const { return _entityHandle != entt::null && _pScene; }

		entt::entity GetHandle() const { return _entityHandle; };
	public:
		template<class T, typename... Args>
		T& AddComponent(Args&&... args) {
			if (HasComponent<T>()) throw std::runtime_error("Entity already has that component!");

			T& component = _pScene->_entityRegistry.emplace<T>(_entityHandle, std::forward<Args>(args)...);
			_pScene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent() {
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			return _pScene->_entityRegistry.get<T>(_entityHandle);
		}

		template<typename T>
		bool HasComponent() {
			return _pScene->_entityRegistry.all_of<T>(_entityHandle);
		}

		template<typename T>
		void RemoveComponent() {
			EXPECT(HasComponent<T>(), "Entity does not have that component!");
			_pScene->_entityRegistry.remove<T>(_entityHandle);
		}

	private:
		entt::entity _entityHandle = entt::null;
		Scene* _pScene = nullptr;
	};
}