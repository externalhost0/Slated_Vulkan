//
// Created by Hayden Rivas on 1/21/25.
//
#pragma once
#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>

#include <vector>

#include "Slate/Common/Debug.h"
#include "Slate/Common/FastSTD.h"
#include "Slate/ECS/Components.h"

namespace Slate {
	class GameEntity final {
	public:
		GameEntity(entt::entity handle, entt::registry& registry) : _handle(handle), _registry(registry) {};
		~GameEntity() = default;
	public:
		const std::string& GetName() const;
		void SetName(const std::string& new_name) const;

		bool HasChildren() const;
		bool HasParent() const;
		void AddChild(GameEntity entity);
		void RemoveChild(GameEntity entity);
		GameEntity GetParent();
		std::vector<GameEntity> GetChildren();
		GameEntity GetRoot();

		inline entt::entity GetHandle() const { return _handle; }
	public:
		bool operator==(const GameEntity& other) const { return _handle == other._handle; }
		bool operator!=(const GameEntity& other) const { return !(*this == other); }
		explicit operator bool() const { return _handle != entt::null; }
	public:
		template<class T>
		bool HasComponent() const {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			return _registry.all_of<T>(_handle);
		}
		template<class T>
		T& GetComponent() {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			return _registry.get<T>(_handle);
		}
		template<class T>
		const T& GetComponent() const {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			return _registry.get<T>(_handle);
		}
		template<class T, typename... Args>
		T& AddComponent(Args&&... args) const {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			if (HasComponent<T>()) throw std::runtime_error("Entity already has that component!");
			return _registry.emplace<T>(_handle, std::forward<Args>(args)...);
		}
		template<class T>
		void RemoveComponent() {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			_registry.remove<T>(_handle);
		}
	private:
		inline CoreComponent& Core() const { return _registry.get<CoreComponent>(_handle); }

		entt::entity _handle;
		entt::registry& _registry;

		friend class Scene;
	};
}









