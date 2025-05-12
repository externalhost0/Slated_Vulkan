//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "ISystem.h"

namespace Slate {
	class RenderSystem : public ISystem {
	public:
		void onStart(Scene& scene) override;
		void onUpdate(Scene& scene) override;
		void onStop(Scene& scene) override;
	};
}