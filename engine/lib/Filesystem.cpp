//
// Created by Hayden Rivas on 1/10/25.
//
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>

#include "Slate/Debug.h"

namespace Slate::Filesystem {
	std::filesystem::path ToDirectory(const std::filesystem::path& path) { return "../../editor" / path; }

	std::filesystem::path GetShaderCacheDirectory() { return ToDirectory((std::string)"shaders/compiled_shaders/"); }

	std::vector<uint32_t> ReadSpvFile(const std::filesystem::path& file_path) {
		std::ifstream file(ToDirectory(file_path), std::ios::in | std::ios::binary);

		EXPECT(file.is_open(), "Failed to open spirv file: {}\n", file_path.string().c_str())

		file.seekg(0, std::ios::end);
		std::streampos size = file.tellg();
		EXPECT(size > 0, "Binary file is empty or has invalid size: {}\n", file_path.c_str())

		std::vector<uint32_t> buffer(size / sizeof (uint32_t));
		file.seekg(0, std::ios::beg);

		file.read(reinterpret_cast<char*>(buffer.data()), size);

		return buffer;
	}
	nlohmann::json ReadJsonFile(const std::filesystem::path& file_path) {
		std::ifstream file(ToDirectory(file_path));
		if (!file.is_open()) {
			fprintf(stderr, "ERROR::Failed to open json file: %s\n", file_path.c_str());
			return {};
		}
		nlohmann::json jsonData = nlohmann::json::parse(file);
		if (jsonData.empty()) {
			fprintf(stderr, "ERROR::Json file is empty: %s\n", file_path.c_str());
			return {};
		}
		return jsonData;
	}

	std::vector<std::byte> ReadBinaryFile(const std::filesystem::path& file_path) {

		std::ifstream file(ToDirectory(file_path), std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			fprintf(stderr, "ERROR::Failed to open binary file: %s\n", file_path.string().c_str());
			return {};
		}

		// get file size
		std::streampos size = file.tellg();
		if (size <= 0) {
			fprintf(stderr, "ERROR::Binary file is empty or has invalid size: %s\n", file_path.c_str());
			return {};
		}

		// resize and move to beginning
		std::vector<std::byte> buffer(size);
		file.seekg(0, std::ios::beg);

		// read into buffer
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			throw std::runtime_error(file_path.string() + ": Failed to read file - " + std::strerror(errno));
		}
		return buffer;
	}

	std::string ReadTextFile(const std::filesystem::path& file_path) {
		std::ifstream file(ToDirectory(file_path));
		if (!file.is_open()) {
			fprintf(stderr, "ERROR::Failed to open text file: %s\n", file_path.c_str());
			return {};
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		if (buffer.str().empty()) {
			fprintf(stderr, "ERROR::Text file is empty: %s\n", file_path.c_str());
			return {};
		}
		return buffer.str();
	}
}














