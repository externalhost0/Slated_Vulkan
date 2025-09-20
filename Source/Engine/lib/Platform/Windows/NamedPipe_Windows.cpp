//
// Created by Hayden Rivas on 5/30/25.
//
#if defined(SLATE_OS_WINDOWS)
#include "Slate/Network/NamedPipe.h"

#include <windows.h>

namespace Slate {
	bool NamedPipe::open(const char* name, bool isServer) {

	}
	bool NamedPipe::write(const char* message) {

	}
	const char* NamedPipe::read() {

	}
	void NamedPipe::close() {

	}
}
#endif