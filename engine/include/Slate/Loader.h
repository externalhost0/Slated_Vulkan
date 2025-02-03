//
// Created by Hayden Rivas on 1/30/25.
//

#pragma once
#include <filesystem>
namespace Slate {
	struct GTLF {

	};

	class Loader {
	public:
		void loadGTLF(const std::filesystem::path& file_path);
	private:
		fastgltf::Parser parser;
	};
}