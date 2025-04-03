//
// Created by Hayden Rivas on 1/21/25.
//
#pragma once
#include <entt/entity/registry.hpp>
#include <entt/entity/entity.hpp>
#include <utility>

#include "FastSTD.h"
#include "Debug.h"
#include "Scene.h"

namespace Slate {
	static constexpr unsigned int MAX_CHILD_COUNT = 128;

	class Entity final {
	public:
		Entity() = default;
		~Entity() = default;
		Entity(const Entity&) = delete;  // disable copy constructor
		Entity& operator=(const Entity&) = delete; // disable assignment operator
		Entity(const entt::entity& handle, WeakPtr<Scene> scene) : _handle(handle), _scene(std::move(scene)) {};
	public:
		bool operator==(const Entity& other) const { return _handle == other._handle; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
		explicit operator bool() const { return _handle != entt::null; }
	public:
		inline std::string GetName() const { return _name; };
		inline entt::entity GetHandle() const { return _handle; };
		inline FastVector<entt::entity, MAX_CHILD_COUNT> GetChildren() { return _childrenhandles; };
		Entity* GetParentPtr();
		Entity& GetRoot();

		void SetName(const std::string& new_name);

		bool HasChildren() const;
		bool HasParent() const;

		void AddChild(const entt::entity& handle);
		void RemoveChild(const entt::entity& handle);
	public:
		template<class T>
		bool HasComponent() const {
			if (StrongPtr<Scene> scene = this->_scene.lock()) {
				return scene->GetRegistry().all_of<T>(_handle);
			}
			return false;
		}
		template<class T>
		T& GetComponent() {
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			if (StrongPtr<Scene> scene = this->_scene.lock()) {
				return scene->GetRegistry().get<T>(_handle);
			}
			throw std::runtime_error("Scene expired!");
		}
		template<class T>
		const T& GetCompnent() const {
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			if (StrongPtr<Scene> scene = this->_scene.lock()) {
				return scene->GetRegistry().get<T>(_handle);
			}
			throw std::runtime_error("Scene expired!");
		}
		template<class T, typename... Args>
		T& AddComponent(Args&&... args) const {
			if (HasComponent<T>()) throw std::runtime_error("Entity already has that component!");
			if (StrongPtr<Scene> scene = this->_scene.lock()) {
				T& comp = scene->GetRegistry().emplace<T>(_handle, std::forward<Args>(args)...);
				// so far we havent had any use of our own functions doing anything special when components are added
//				scene->OnComponentAdded<T>(this, comp);
				return comp;
			}
			throw std::runtime_error("Scene expired!");
		}
		template<class T>
		void RemoveComponent() {
			if (!HasComponent<T>()) throw std::runtime_error("Entity does not have that component!");
			if (StrongPtr<Scene> scene = this->_scene.lock()) {
				scene->GetRegistry().remove<T>(_handle);
				return;
			}
			throw std::runtime_error("Scene expired!");
		}
	private:
		void SetParent(Entity* parent);

		std::string _name = "Null Name";
		entt::entity _handle = entt::null;

		WeakPtr<Scene> _scene;
		Optional<Entity*> _parent = std::nullopt;
		FastVector<entt::entity, MAX_CHILD_COUNT> _childrenhandles;
	};
}