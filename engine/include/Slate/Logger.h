//
// Created by Hayden Rivas on 3/17/25.
//

#pragma once
#include <fmt/printf.h>
#include <fmt/format.h>
#include <exception>
namespace Slate {
	enum class LogLevel : uint8_t {
		Info,
		Warning,
		Error,
	};
	inline std::string LogLevelToString(LogLevel level) {
		switch (level) {
			case LogLevel::Info:    return "INFO";
			case LogLevel::Warning: return "WARNING";
			case LogLevel::Error:   return "ERROR";
			default:                return "UNKNOWN";
		}
	}
	#define LOG_EXCEPTION(exception) fmt::println(stderr, "[EXCEPTION] | {}", exception.what())
	#define LOG_USER(level, message, ...) fmt::println(stderr, "[{}] \"{}\" | {}", LogLevelToString(level), __FUNCTION__, fmt::format(message __VA_OPT__(, __VA_ARGS__)))
	#define RUNTIME_ERROR(message, ...) std::runtime_error(fmt::format(message __VA_OPT__(, __VA_ARGS__)))
}