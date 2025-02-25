//
// Created by Hayden Rivas on 2/4/25.
//

#pragma once
#include <string>

namespace Slate {
	class GameObject {
	public:
		GameObject();
		~GameObject();

		void OnInitialize();
		void OnPlay();
		void OnTick();
		void OnStop();
		void OnDestruction();

		bool IsActive();
		void SetActive();



		void Destroy();
	private:
		std::string _name;
		bool _isActive;

	};
}