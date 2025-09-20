//
// Created by Hayden Rivas on 1/21/25.
//
#pragma once
#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>

#include <vector>

#include "Slate/Common/FastSTD.h"
#include "Slate/Common/HelperMacros.h"
#include "Slate/ECS/Components.h"

namespace Slate {
	class GameEntity final {
		struct Name {
			std::string name = "Unnamed";
		};
		struct Active {
			bool active = true;
		};
		struct Hierarchy {
			entt::entity parent = entt::null;
			FastVector<entt::entity, MAX_CHILD_COUNT> children = {};
		};
	public:
		GameEntity(entt::entity handle, entt::registry& registry) : _handle(handle), _registry(registry) {};
		~GameEntity() = default;
	public:
		const std::string& getName() const;
		void setName(const std::string& new_name);

		bool hasChildren() const;
		bool hasParent() const;

		void addChild(GameEntity entity);
		void removeChild(GameEntity entity);

		GameEntity getParent();
		GameEntity getRoot();
		std::vector<GameEntity> getChildren();

		inline entt::entity getHandle() const { return _handle; }
	public:
		bool operator==(const GameEntity& other) const { return _handle == other._handle; }
		bool operator!=(const GameEntity& other) const { return !(*this == other); }
		explicit operator bool() const { return _handle != entt::null; }
	public:
		template<class T>
		bool hasComponent() const {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			return _registry.all_of<T>(_handle);
		}
		template<class T>
		T& getComponent() {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			if (!hasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			return _registry.get<T>(_handle);
		}
		template<class T>
		const T& getComponent() const {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			if (!hasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			return _registry.get<T>(_handle);
		}
		template<class T, typename... Args>
		T& addComponent(Args&&... args) const {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			if (hasComponent<T>()) throw std::runtime_error("Entity already has that component!");
			return _registry.emplace<T>(_handle, std::forward<Args>(args)...);
		}
		template<class T>
		void removeComponent() {
			static_assert(std::is_base_of<ComponentBase, T>::value, "T must inherit from ComponentBase");
			if (!hasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			_registry.remove<T>(_handle);
		}
	private:
		entt::entity _handle;
		entt::registry& _registry;

		friend class Scene;
	};
}









