//
// Created by Hayden Rivas on 3/16/25.
//
#include "Slate/Resources/ScriptResource.h"

#include "Slate/Filesystem.h"

#include <nfd.h>

namespace Slate {
	Result ScriptResource::_loadResourceImpl(const std::filesystem::path& path) {
		auto out = Filesystem::ReadTextFile(path);


		return Result::SUCCESS;
	}

}