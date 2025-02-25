//
// Created by Hayden Rivas on 1/11/25.
//

#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Slate::Filesystem {
	std::filesystem::path ToDirectory(const std::filesystem::path& path);
	std::filesystem::path GetShaderCacheDirectory();

	// read files
	std::vector<uint32_t> ReadSpvFile(const std::filesystem::path& file_path);
	nlohmann::json ReadJsonFile(const std::filesystem::path& file_path);
	std::vector<std::byte> ReadBinaryFile(const std::filesystem::path& file_path);
	std::string ReadTextFile(const std::filesystem::path& file_path);
}