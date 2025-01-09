//
// Created by Hayden Rivas on 1/8/25.
//

#pragma once

namespace Slate {
	class IManager {
	public:
		IManager() = default;
		virtual ~IManager() = default;
	private:
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
	};
}