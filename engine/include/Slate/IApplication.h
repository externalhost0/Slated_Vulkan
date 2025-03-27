//
// Created by Hayden Rivas on 11/27/24.
//
#pragma once
#include "Slate/SmartPointers.h"
#include "Slate/SystemManager.h"

namespace Slate {
	// application, topmost system of Slate engine
	class IApplication {
	public:
		// used to make the users app!
		template <typename Derived>
		static Unique<IApplication> Create() {
			static_assert(std::is_base_of<IApplication, Derived>::value, "Derived must inherit from Application");
			static bool instanceCreated;
			if (instanceCreated) throw std::runtime_error("An instance of Application has already been created!");
			instanceCreated = true;
			return CreateUnique<Derived>();
		}

		const SystemManager& GetSystems() const { return this->manager; }
		// starts the app, should be done in main() after creation
		virtual void Run() final; // (init(), loop(), end())
		virtual void StopLoop() final;
	protected:
		SystemManager manager;
	private:
		bool continue_loop = true;
	public:
		IApplication(const IApplication &) = delete; // delete copy construction
		virtual IApplication& operator=(const IApplication&) = delete; // delete assignment construction
	protected:
		// disallows instancing IApplication and requires user to inherit it themselves
		IApplication() = default;
		virtual ~IApplication() = default;
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

