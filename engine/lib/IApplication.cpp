//
// Created by Hayden Rivas on 1/7/25.
//

#include "Slate/IApplication.h"

#include "Slate/Systems/InputSystem.h"
#include "Slate/Systems/RenderSystem.h"
#include "Slate/Systems/TimeSystem.h"
#include "Slate/Systems/WindowSystem.h"

namespace Slate {
	// cause im lazy and want this in one call
	void IApplication::Run() {
		this->BaseSlateApp_Start();
		this->BaseSlateApp_Loop();
		this->BaseSlateApp_End();
	}
	void IApplication::StopLoop() {
		this->continue_loop = false;
	}
	void IApplication::BaseSlateApp_Start() {
		manager.RegisterSystem<TimeSystem>();
		manager.RegisterSystem<WindowSystem>();
		manager.RegisterSystem<InputSystem>();
		manager.RegisterSystem<RenderSystem>();

		this->Initialize(); // user start func
	}
	void IApplication::BaseSlateApp_Loop() {
		while (continue_loop) {
			manager.GetSystem<TimeSystem>()->UpdateTime();
			this->Loop(); // user loop func
		}
	}
	void IApplication::BaseSlateApp_End() {
		this->Shutdown(); // user end func

		manager.GetSystem<RenderSystem>()->Shutdown();
		manager.GetSystem<InputSystem>()->Shutdown();
		manager.GetSystem<WindowSystem>()->Shutdown();
		manager.GetSystem<TimeSystem>()->Shutdown();
	}
}