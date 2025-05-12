//
// Created by Hayden Rivas on 3/17/25.
//

#pragma once
#include <fmt/printf.h>
#include <fmt/format.h>
#include <exception>
namespace Slate {

	enum class LogType : uint8_t {
		Info = 0,
		Warning,
		Error,
		FatalError
	};
	constexpr const char* level_strings[] = {
			"Info",
			"Warning",
			"Error",
			"FatalError"
	};
	constexpr const char* GetLogLevelAsString(LogType level) {
		return level_strings[(int)level];
	}
	#define LOG_EXCEPTION(exception) fmt::println(stderr, "[EXCEPTION] | {}", exception.what())
	#define LOG_USER(level, message, ...) fmt::println(stdout, "[{}] Function: \"{}\" | {}", GetLogLevelAsString(level), __FUNCTION__, fmt::format(message __VA_OPT__(, __VA_ARGS__)))
	#define RUNTIME_ERROR(message, ...) std::runtime_error(fmt::format(message __VA_OPT__(, __VA_ARGS__)))
}