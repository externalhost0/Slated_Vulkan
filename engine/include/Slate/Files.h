//
// Created by Hayden Rivas on 1/11/25.
//

#pragma once
#include <string>
#include <filesystem>

namespace Slate {
	std::string ToDirectory(const std::string& path);

	// read files
	std::vector<std::byte> ReadBinaryFile(const std::filesystem::path& file_path);
	std::string ReadTextFile(const std::filesystem::path& file_path);
}