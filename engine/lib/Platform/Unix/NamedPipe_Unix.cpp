//
// Created by Hayden Rivas on 5/30/25.
//
#if defined(SLATE_OS_MACOS)
#include "Slate/Common/HelperMacros.h"
#include "Slate/Network/NamedPipe.h"

#include <cstdio>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <zpp_bits.h>

namespace Slate {
	constexpr unsigned int MAX_BUFFER = 1024;
	struct NamedPipe::Impl {
		int fd = -1;
		const char* filePath{};
	};

	NamedPipe::NamedPipe() {
		_impl = CreateUniquePtr<NamedPipe::Impl>();
	}
	NamedPipe::~NamedPipe() {
		close();
	}

	bool NamedPipe::open(const char* name, bool isServer) {
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "/tmp/%s", name);
		_impl->filePath = strdup(buffer);
		if (isServer) {
			mkfifo(buffer, 0666); // read and write but not execute
		}
		_impl->fd = ::open(buffer, O_RDWR);
		return _impl->fd != -1;
	}
	bool NamedPipe::write(const char* message) {
		ASSERT_MSG(_impl->fd > -1, "Pipe must be open before it can be written to!");
		uint32_t len = (uint32_t)strlen(message);
		if (::write(_impl->fd, &len, sizeof(len)) != sizeof(len)) return false;
		return ::write(_impl->fd, message, len) == (ssize_t)len;
	}
	const char* NamedPipe::read() {
		ASSERT_MSG(_impl->fd > -1, "Pipe must be open before it can be read from!");

		static char buffer[1024];
		uint32_t len = 0;
		if (::read(_impl->fd, &len, sizeof(len)) != sizeof(len)) return "NO OUPUT TO CAPTURE";
		// trim incase of overflow
		if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;

		// over multiple packets
		ssize_t total = 0;
		while (total < len) {
			ssize_t bytes = ::read(_impl->fd, buffer + total, len - total);
			if (bytes <= 0) break;
			total += bytes;
		}
		buffer[total] = '\0';
		return buffer;
	}
	void NamedPipe::close() {
		::close(_impl->fd);
		_impl->fd = -1;
		_impl->filePath = nullptr;
	}
}
#endif