//
// Created by Hayden Rivas on 1/9/25.
//
#pragma once
#include <memory>
namespace Slate {
	// alias for std::shared_ptr
	template<typename T>
	using Scope = std::unique_ptr<T>;

	// for populating the shared_ptr
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args &&... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}