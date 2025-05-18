//
// Created by Hayden Rivas on 4/1/25.
//

#pragma once
#include <cstddef>
#include <stdexcept>
#include <algorithm>

namespace Slate {
	template <typename T, size_t MaxSize>
	class FastVector {
	public:
		// constructors
		// two others are used to copy vector into a FastVector
		constexpr FastVector() : _count(0) {}
		explicit FastVector(const std::vector<T>& vec) : _count(0) {
			if (vec.size() > MaxSize) {
				throw std::length_error("std::vector size exceeds FastVector capacity");
			}
			_count = vec.size();
			std::copy(vec.begin(), vec.end(), _data);
		}
		explicit FastVector(std::vector<T>&& vec) : _count(0) {
			if (vec.size() > MaxSize) {
				throw std::length_error("std::vector size exceeds FastVector capacity");
			}
			_count = vec.size();
			std::move(vec.begin(), vec.end(), _data);
		}
		// assignment
		FastVector& operator=(const std::vector<T>& vec) {
			if (vec.size() > MaxSize) {
				throw std::length_error("std::vector size exceeds FastVector capacity");
			}
			_count = vec.size();
			std::copy(vec.begin(), vec.end(), _data);
			return *this;
		}
		FastVector& operator=(std::vector<T>&& vec) {
			if (vec.size() > MaxSize) {
				throw std::length_error("std::vector size exceeds FastVector capacity");
			}
			_count = vec.size();
			std::move(vec.begin(), vec.end(), _data);
			return *this;
		}

		// addition
		bool push_back(const T& element) noexcept {
			if (_count < MaxSize) {
				_data[_count++] = element;
				return true;
			}
			return false;
		}
		bool push_back(T&& element) noexcept {
			if (_count < MaxSize) {
				_data[_count++] = std::move(element);
				return true;
			}
			return false;
		}
		template <typename... Args>
		void emplace_back(Args&&... args) {
			if (_count < MaxSize) {
				_data[_count++] = T(std::forward<Args>(args)...);
			}
		}
		bool insert(size_t index, const T& element) noexcept {
			if (_count >= MaxSize || index > _count) {
				return false;
			}
			for (size_t i = _count; i > index; --i) {
				_data[i] = std::move(_data[i - 1]);
			}
			_data[index] = element;
			++_count;
			return true;
		}
		bool insert(size_t index, T&& element) noexcept {
			if (_count >= MaxSize || index > _count) {
				return false;
			}
			for (size_t i = _count; i > index; --i) {
				_data[i] = std::move(_data[i - 1]);
			}
			_data[index] = std::move(element);
			++_count;
			return true;
		}

		// fast removal
		void remove(size_t index) noexcept {
			if (index < _count) {
				_data[index] = std::move(_data[--_count]);
			}
		}
		// order preserving removal
		void erase(size_t index) noexcept {
			if (index >= _count) return;
			for (size_t i = index; i < _count - 1; ++i) {
				_data[i] = std::move(_data[i + 1]);
			}
			--_count;
		}
		void erase(T* ptr) noexcept {
			size_t index = static_cast<size_t>(ptr - _data);
			erase(index);
		}
		void erase_value(const T& value) noexcept {
			for (size_t i = 0; i < _count; ++i) {
				if (_data[i] == value) {
					erase(i);
				}
			}
		}

		// indice retrieval
		T& operator[](size_t index) noexcept { return _data[index]; }
		const T& operator[](size_t index) const noexcept { return _data[index]; }
		// total retrieval
		T* data() noexcept { return _data; }
		const T* data() const noexcept { return _data; }

		// helpers
		size_t size() const noexcept { return _count; }
		constexpr size_t capacity() const noexcept { return MaxSize; }
		bool empty() const noexcept { return _count == 0; }
		void clear() noexcept { _count = 0; }
		void swap(size_t first_index, size_t second_index) noexcept {
			T temp = _data[first_index];
			_data[first_index] = _data[second_index];
			_data[second_index] = temp;
		}

		// iteration
		template <typename F>
		void for_each(F&& func) noexcept(noexcept(func(std::declval<T&>()))) {
			for (size_t i = 0; i < _count; ++i) {
				func(_data[i]);
			}
		}
		T* begin() { return _data; }
		T* end() { return _data + _count; }
		const T* begin() const { return _data; }
		const T* end() const { return _data + _count; }
	private:
		T _data[MaxSize];
		size_t _count;
	};



	template <typename T, size_t MaxSize>
	class FastQueue {
	public:
		constexpr FastQueue() : _front(0), _count(0) {};

		// addition
		void push(const T& element) noexcept {
			if (_count < MaxSize) {
				size_t backIdx = (_front + _count) % MaxSize;
				_data[backIdx] = element;
				++_count;
			}
		}
		void push(T&& element) noexcept {
			if (_count < MaxSize) {
				size_t backIdx = (_front + _count) % MaxSize;
				_data[backIdx] = std::move(element);
				++_count;
			}
		}
		template <typename... Args>
		void emplace(Args&&... args) noexcept {
			if (_count < MaxSize) {
				size_t backIdx = (_front + _count) % MaxSize;
				_data[backIdx] = T(std::forward<Args>(args)...);
				++_count;
			}
		}
		// retrieval
		T& front() noexcept { return _data[_front]; }
		const T& front() const noexcept { return _data[_front]; }
		T& back() noexcept { return _data[(_front + _count - 1) % MaxSize]; }
		const T& back() const noexcept { return _data[(_front + _count - 1) % MaxSize]; }

		// removal
		void pop() noexcept {
			if (_count > 0) {
				_front = (_front + 1) % MaxSize;
				--_count;
			}
		}

		// helpers
		size_t size() const noexcept { return _count; }
		constexpr size_t capacity() const noexcept { return MaxSize; }
		bool empty() const noexcept { return _count == 0; }
	private:
		T _data[MaxSize];
		size_t _count;
		size_t _front;
	};
}