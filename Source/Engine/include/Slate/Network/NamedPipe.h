//
// Created by Hayden Rivas on 5/30/25.
//

#pragma once
#include <Slate/SmartPointers.h>

namespace Slate {
	class NamedPipe {
	public:
		NamedPipe();
		~NamedPipe();

		bool open(const char* name, bool isServer);
		bool write(const char* message);
		const char* read();
		void close();
	private:
		struct Impl;
		UniquePtr<Impl> _impl;
	};
}