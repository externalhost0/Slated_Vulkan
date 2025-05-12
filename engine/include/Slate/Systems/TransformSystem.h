//
// Created by Hayden Rivas on 4/7/25.
//

#pragma once
#include "ISystem.h"


namespace Slate {
	class TransformSystem : public ISystem {
	public:
		void onStart(Scene& scene) override;
		void onUpdate(Scene& scene) override;
		void onStop(Scene& scene) override;
	};
}