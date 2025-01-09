//
// Created by Hayden Rivas on 1/8/25.
//
#pragma once

#include <Slate/Application.h>

namespace Slate {
	class EditorApplication : public Application {
	public:

	private:
		void Initialize() override;
		void Loop() override;
		void Shutdown() override;
	};
}
