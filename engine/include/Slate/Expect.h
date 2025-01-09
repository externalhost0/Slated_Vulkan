//
// Created by Hayden Rivas on 1/8/25.
//

#pragma once

#include <cstdio>
#include <csignal>
#include <functional>

namespace Slate {
	// our error macro
#define EXPECT(ERROR, FORMAT, ...) {                                                                                                        \
    int macroErrorCode = ERROR;                                                                                                             \
    if(!macroErrorCode) {                                                                                                                   \
        fprintf(stderr, "EXPECT: %s:%i -> %s -> Error(%i):\n\t" FORMAT "\n", __FILE__, __LINE__, __func__, macroErrorCode, ##__VA_ARGS__);  \
        raise(SIGABRT);                                                                                                                     \
    }                                                                                                                                       \
}
}
