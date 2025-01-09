//
// Created by Hayden Rivas on 11/27/24.
//
#pragma once

#include "Input.h"
#include "Window.h"

#include <unordered_map>

namespace Slate {
	// used to retrieve systems without singleton application
	class SystemLocator {
	public:
		// for registration
		template <typename T>
		static void Provide(T* service) {
			GetSystems()[typeid(T).hash_code()] = service;
		}
		// for retrieval
		template <typename T>
		static T& Get() {
			return *static_cast<T*>(GetSystems()[typeid(T).hash_code()]);
		}
	private:
		static std::unordered_map<size_t, void*>& GetSystems() {
			static std::unordered_map<size_t, void*> services;
			return services;
		}
	};

	// application, top most system of Slate engine
	class Application {
	public:
		// used to make the users app!
		template <typename Derived>
		static std::unique_ptr<Application> Create() {
			static_assert(std::is_base_of<Application, Derived>::value, "Derived must inherit from Application");

			static bool instanceCreated = false;
			if (instanceCreated)
				throw std::runtime_error("An instance of Application has already been created!");
			instanceCreated = true;

			return std::make_unique<Derived>();
		}
		// starts the app, should be done in main() after creation
		void Run(); // (init(), loop(), end())

	public:
		bool shouldStopLoop = false;
		virtual ~Application() = default;
		Application(const Application&) = delete; // delete copy construction
		virtual Application& operator=(const Application&) = delete; // delete assignment construction
	protected:
		// user defined run event sequence
		// this is where the user will write all their code
		// therefore it means a Slate app requires inheriting a Application class
		// not pure virtual ig cause the user cannot do anything i guess
		virtual void Initialize() {};
		virtual void Loop()       {};
		virtual void Shutdown()   {};

		// privte constructor means users are required to "create" an instance via static functions
		// makes it so there is ONLY one possible instance
		// also indicates that creating an instance doesnt do anything until run()
		Application() {
			// very important to register the managers to the system provider for access throughout the application easily
			SystemLocator::Provide<InputManager>(&MInputManager);
			SystemLocator::Provide<WindowManager>(&MWindowManager);
		};
	private:
		// single instance managers
		// explicit empty initalization so the analysis' warning disappears
		InputManager MInputManager   = {};
		WindowManager MWindowManager = {};

		// general system event steps, only for base Application engine to touch
		// this is the sequence that exists outside of the users control
		void BaseSlateApp_Start();
		void BaseSlateApp_Loop();
		void BaseSlateApp_End();
	};
}

