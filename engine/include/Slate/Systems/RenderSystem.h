//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "ISystem.h"
#include "Slate/RenderEngine.h"

namespace Slate {
	class RenderSystem : public ISystem {
	public:
		RenderEngine& GetEngine() { return this->engine; }

		void Render();
		void Update();
	private:
		void StartupImpl() override;
		void ShutdownImpl() override;
		RenderEngine engine;
	};
}