//
// Created by Hayden Rivas on 1/10/25.
//
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace Slate {
	[[maybe_unused]]
	std::string ToDirectory(const std::string& filepath) {
		return "../../editor/" + filepath;
	}

	std::vector<std::byte> ReadBinaryFile(const std::filesystem::path& file_path) {
		std::string adjustedfilepath = ToDirectory(file_path);

		std::ifstream file(adjustedfilepath, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			fprintf(stderr, "ERROR::Failed to open binary file: %s\n", file_path.string().c_str());
			return {};
		}

		// get file size
		std::streamsize size = file.tellg();
		if (size <= 0) {
			fprintf(stderr, "ERROR::Binary file is empty or has invalid size: %s\n", file_path.string().c_str());
			return {};
		}

		// resize and move to beginning
		std::vector<std::byte> buffer(static_cast<size_t>(size));
		file.seekg(0, std::ios::beg);

		// read into buffer
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			throw std::runtime_error(file_path.string() + ": Failed to read file - " + std::strerror(errno));
		}
		return buffer;
	}

	std::string ReadTextFile(const std::filesystem::path& file_path) {
		std::ifstream file(file_path);
		if (file.is_open()) {
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














