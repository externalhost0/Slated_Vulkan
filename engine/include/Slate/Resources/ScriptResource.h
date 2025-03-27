//
// Created by Hayden Rivas on 3/19/25.
//

#pragma once
#include "IResource.h"
namespace Slate {
	struct ScriptResource : public IResource {
	public:

	private:
		Result LoadResourceImpl(const std::filesystem::path& path) override;
	};

}