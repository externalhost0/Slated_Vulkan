//
// Created by Hayden Rivas on 1/14/25.
//

#pragma once

namespace Slate {
	class BaseSystem {
	public:
		BaseSystem() = default;
		virtual ~BaseSystem() = default;
	public:
		bool _isInitialized = false;
	private:
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	};
}