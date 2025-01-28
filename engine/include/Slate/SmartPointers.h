//
// Created by Hayden Rivas on 1/22/25.
//

#pragma once
#include <memory>
namespace Slate {
	// alias for std::shared_ptr
	template<typename T>
	using Shared = std::shared_ptr<T>;

	// for populating the shared_ptr
	template<typename T, typename ... Args>
	constexpr Shared<T> CreateShared(Args &&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// alias for std::unique_ptr
	template<typename T>
	using Unique = std::unique_ptr<T>;

	// for populating the unique_ptr
	template<typename T, typename ... Args>
	constexpr Unique<T> CreateUnique(Args &&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}