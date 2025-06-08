//
// Created by Hayden Rivas on 5/18/25.
//

#pragma once

#include <vector>
#include <cstdint>
#include <filesystem>

#include "Slate/Common/Logger.h"
#include "Slate/Common/Invalids.h"

namespace Slate {
	// forward delcare all this stuff
	class IResource;
	class MeshResource;
	class ScriptResource;
	class ShaderResource;
	class AudioResource;
	class TextureResource;

	template<typename Type>
	class ResourceHandle {
	public:
		ResourceHandle() = default;
		ResourceHandle(uint32_t idx, uint32_t gen) : _index(idx), _gen(gen) {};
	public:
		uint32_t index() const { return _index; }
		uint32_t gen() const { return _gen; }
		bool valid() const { return _gen != 0; }
	private:
		uint32_t _index = 0;
		uint32_t _gen = 0;
	};

	// all the resources must be defined here and in the forward declaration block at the top
	using MeshHandle = ResourceHandle<MeshResource>;
	using ScriptHandle = ResourceHandle<ScriptResource>;
	using ShaderHandle = ResourceHandle<ShaderResource>;
	using AudioHandle = ResourceHandle<AudioResource>;
	using ImageHandle = ResourceHandle<TextureResource>;

	template<class T>
	class ResourcePool {
		static_assert(std::is_base_of_v<IResource, T>, "T must inherit from IResource");
		struct ResourceEntry {
			explicit ResourceEntry(T&& obj) : _obj(std::move(obj)) {}
			T _obj = {};
			uint32_t _gen = 1;
			uint32_t _nextFreeIndex = Invalid<uint32_t>;
		};
	public:
		ResourceHandle<T> create(const std::filesystem::path& filepath) {
			uint32_t idx;

			// perform standard operations on our resource before storage
			T obj;
			obj.loadResource(filepath);

			// if: the first free isnt set (we have no available free slots!)
			// else: we have slots that could be freed
			if (_firstFreeIndex == Invalid<uint32_t>) {
				// create new allocation space for vector
				idx = static_cast<uint32_t>(_objects.size());
				_objects.emplace_back(std::move(obj));
			} else {
				// reuse vector's allocated space
				idx = _firstFreeIndex;
				_firstFreeIndex = _objects[idx]._nextFreeIndex;
				_objects[idx]._obj = std::move(obj);
			}
			_numObjects++;
			return ResourceHandle<T>{idx, _objects[idx]._gen};
		}
		void destroy(ResourceHandle<T> id) {
			if (!id.valid()) {
				LOG_USER(LogType::Warning, "Request to 'destroy' invalid handle intercepted.");
				return;
			}
			const uint32_t index = id.index();
			assert(index < _objects.size());
			assert(id.gen() == _objects[index]._gen);

			// destruction made need to be rewritten
			_objects[index]._obj = T{};
			_objects[index]._gen++;
			_objects[index]._nextFree = _firstFreeIndex;
			_firstFreeIndex = index;
			_numObjects--;
		}
		// retrieval
		T* get(ResourceHandle<T> id) {
			if (!id.valid()) {
				LOG_USER(LogType::Warning, "Request to 'get' invalid handle intercepted.");
				return nullptr;
			}
			const uint32_t index = id.index();
			if (index >= _objects.size()) return nullptr;
			if (id.gen() != _objects[index]._gen) return nullptr;
			return &_objects[index]._obj;
		}
		// const retrieval
		const T* get(ResourceHandle<T> id) const {
			if (!id.valid()) {
				LOG_USER(LogType::Warning, "Request to 'get' invalid handle intercepted.");
				return nullptr;
			}
			const uint32_t index = id.index();
			if (index >= _objects.size()) return nullptr;
			if (id.gen() != _objects[index]._gen) return nullptr;
			return &_objects[index]._obj;
		}
		uint32_t getNumActiveSlots() const { return _numObjects; }
		uint32_t getNumAllocatedSlots() const { return _objects.size(); }
	private:
		uint32_t _firstFreeIndex = Invalid<uint32_t>;
		uint32_t _numObjects = 0;
		std::vector<ResourceEntry> _objects;
	};
}