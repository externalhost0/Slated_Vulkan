//
// Created by Hayden Rivas on 11/27/24.
//
#pragma once

#include "Input.h"
#include "Window.h"
#include "Renderer.h"
#include "SmartPointers.h"

#include <memory>

namespace Slate {
	// application, topmost system of Slate engine
	class Application {
	public:
		// used to make the users app!
		template <typename Derived>
		static std::unique_ptr<Application> Create() {
			static_assert(std::is_base_of<Application, Derived>::value, "Derived must inherit from Application");

			static bool instanceCreated = false;
			if (instanceCreated) throw std::runtime_error("An instance of Application has already been created!");
			instanceCreated = true;

			return std::make_unique<Derived>();
		}
		// starts the app, should be done in main() after creation
		void Run(); // (init(), loop(), end())
		bool continue_Loop = true;
	public:
		Application() = default;
		virtual ~Application() = default;
		Application(const Application&) = delete; // delete copy construction
		virtual Application& operator=(const Application&) = delete; // delete assignment construction
	protected:
		// user defined run event sequence
		// this is where the user will write all their code
		// therefore it means a Slate app requires inheriting a Application class
		// not pure virtual ig cause the user should be able to do anything i guess
		virtual void Initialize() {};
		virtual void Loop()       {};
		virtual void Shutdown()   {};
	private:
		// general system event steps, only for base Application engine to touch
		// this is the sequence that exists outside of the users control
		void BaseSlateApp_Start();
		void BaseSlateApp_Loop();
		void BaseSlateApp_End();
	};
}

