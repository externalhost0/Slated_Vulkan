//
// Created by Hayden Rivas on 1/8/25.
//

#pragma once

#include <cstdio>
#include <csignal>
#include <functional>

#include <fmt/core.h>
#include <fmt/format.h>

namespace Slate {

#ifdef SLATE_DEBUG
#define EXPECT(ERROR, FORMAT, ...) {                                                                                                            \
    int macroErrorCode = static_cast<int>(ERROR);                                                                                               \
    if (!macroErrorCode) {                                                                                                                      \
        fmt::print(stderr, "EXPECT: {}:{} -> {} -> Error({}):\n\t" FORMAT "\n", __FILE__, __LINE__, __func__, macroErrorCode, ##__VA_ARGS__);   \
        std::raise(SIGABRT);                                                                                                                    \
    }                                                                                                                                           \
}
#else
#define EXPECT(ERROR, FORMAT)
#endif



}
