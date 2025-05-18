//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "IResource.h"
namespace Slate {
	struct ScriptResource : public IResource {
	public:
		void loadAssembly();
		void unloadAssembly();
	private:
		Result _loadResourceImpl(const std::filesystem::path& path) override;
	};

}