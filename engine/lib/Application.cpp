//
// Created by Hayden Rivas on 1/7/25.
//

#include "Slate/Application.h"
#include "Slate/Time.h"

namespace Slate {
	// cause im lazy and want this in one call
	void Application::Run() {
		this->BaseSlateApp_Start();
		this->BaseSlateApp_Loop();
		this->BaseSlateApp_End();
	}

	void Application::BaseSlateApp_Start() {
		Time::Initialize();
		{
			MWindowManager.Initialize();
			MInputManager.Initialize();

		}

		this->Initialize(); // user start func
	}
	void Application::BaseSlateApp_Loop() {
		while (!shouldStopLoop) {
			Time::UpdateDeltaTime(); // Time class is one of the few that has no need to be tied to anything
			this->Loop(); // user loop func
		}
	}
	void Application::BaseSlateApp_End() {
		this->Shutdown(); // user end func
		{
			MInputManager.Shutdown();
			MWindowManager.Shutdown();
		}
	}
}