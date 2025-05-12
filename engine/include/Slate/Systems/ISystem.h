//
// Created by Hayden Rivas on 1/14/25.
//

#pragma once
#include <cstdint>

namespace Slate {
	class SystemManager;
	class Scene;

	enum class SystemState : uint8_t {
		Inactive = 0,
		Starting,
		Updating,
		Stopping,
	};

	class ISystem {
	public:
		virtual ~ISystem() = default;

		virtual void start(Scene& scene) final;
		virtual void update(Scene& scene) final;
		virtual void stop(Scene& scene) final;

		virtual SystemState getState() final;
		virtual const char* getStateAsString() final;
	protected:
		// to be overriden
		virtual void onStart(Scene& scene) = 0;
		virtual void onUpdate(Scene& scene) = 0;
		virtual void onStop(Scene& scene) = 0;
	private:
		SystemState currentState = SystemState::Inactive;
	};
}