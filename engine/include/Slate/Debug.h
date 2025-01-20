//
// Created by Hayden Rivas on 1/13/25.
//

#pragma once
#include <fmt/core.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>
#include <csignal>

namespace Slate {

#ifdef SLATE_DEBUG
#define VK_CHECK(x) \
    do { \
        VkResult errorResult = x; \
        if (errorResult) { \
			fmt::print(stderr, "[VULKAN] Detected Vulkan error: {}:{} -> {} \n\t {}", __FILE__, __LINE__, __func__, string_VkResult(errorResult)); \
            std::raise(SIGABRT); \
		} \
    } while (0)
#else
#define VK_CHECK(x) x
#endif

}