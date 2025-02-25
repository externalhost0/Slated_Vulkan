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

#ifdef SLATE_DEBUG
#define EXPECT(ERROR, FORMAT, ...) {                                                                                                            		   \
    int macroErrorCode = static_cast<int>(ERROR);                                                                                               		   \
    if (!macroErrorCode) {                                                                                                                   		 	   \
        fmt::print(stderr, "EXPECT: {}:{} -> {} -> Error({}):\n\t" FORMAT "\n", __FILE__, __LINE__, __func__, macroErrorCode __VA_OPT__(, __VA_ARGS__));   \
        std::raise(SIGABRT);                                                                                                                    		   \
    }                                                                                                                                           		   \
}
#else
#define EXPECT(ERROR, FORMAT)
#endif


}