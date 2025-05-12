//
// Created by Hayden Rivas on 1/13/25.
//

#pragma once
#include <csignal>
#include <cstdio>
#include <functional>

#include <fmt/format.h>

namespace Slate {

#ifdef SLATE_DEBUG
#define VK_CHECK(x) \
    do { \
        VkResult errorResult = x; \
        if (errorResult) { \
			fmt::print(stderr, "[VULKAN] Detected Vulkan error: {}:{} -> {} \n\t", __FILE__, __LINE__, __func__); \
            std::raise(SIGABRT); \
		} \
    } while (0)
#else
#define VK_CHECK(x) x
#endif

// this macro is not meant to be used outside of this header file
#if defined(_MSC_VER)
#define ASSUME_(cond) __assume(cond)
#elif defined(__clang__) || defined(__GNUC__)
#define ASSUME_(cond) if (!(cond)) __builtin_unreachable()
#else
#define ASSUME_(cond) ((void)0)
#endif


#ifdef SLATE_DEBUG
#define ASSERT_MSG(ERROR, FORMAT, ...) do { \
    if (!(ERROR)) { \
        fmt::print(stderr, "[ASSERT] Assertion failed: {}:{} -> {} -> Error:\n\t" FORMAT "\n", __FILE__, __LINE__, __func__ __VA_OPT__(,) __VA_ARGS__); \
        std::raise(SIGABRT); \
    } \
	ASSUME_(ERROR); \
} while(0)
#else
#define ASSERT_MSG(ERROR, FORMAT)
#endif


#ifdef SLATE_DEBUG
#define ASSERT(ERROR) do { \
    if (!(ERROR)) { \
        fmt::print(stderr, "[ASSERT] Assertion failed: {}:{} -> {}\n", __FILE__, __LINE__, __func__); \
        std::raise(SIGABRT); \
    } \
    ASSUME_(ERROR); \
} while(0)
#else
#define ASSERT(ERROR)
#endif

}