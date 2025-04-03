//
// Created by Hayden Rivas on 1/22/25.
//

#pragma once
#include <memory>
#include <optional>
namespace Slate {
	// alias for std::shared_ptr
	template<typename T>
	using StrongPtr = std::shared_ptr<T>;
	// for populating the shared_ptr
	template<typename T, typename ... Args>
	constexpr StrongPtr<T> CreateStrongPtr(Args &&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// alias for std::weak_ptr
	template<typename T>
	using WeakPtr = std::weak_ptr<T>;
	// for populating the weak_ptr
	template<typename T, typename ... Args>
	constexpr WeakPtr<T> CreateWeakPtr(Args &&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// alias for std::unique_ptr
	template<typename T>
	using UniquePtr = std::unique_ptr<T>;
	// for populating the unique_ptr
	template<typename T, typename ... Args>
	constexpr UniquePtr<T> CreateUniquePtr(Args &&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	// alias for std::optional
	template<typename T>
	using Optional = std::optional<T>;
	// for populating the unique_ptr
	template<typename T, typename ... Args>
	constexpr Optional<T> CreateOptional(Args &&... args) {
		return std::make_optional<T>(std::forward<Args>(args)...);
	}

}