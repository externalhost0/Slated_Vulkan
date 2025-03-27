//
// Created by Hayden Rivas on 1/10/25.
//
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <nlohmann/json.hpp>

#include "Slate/Filesystem.h"
#include "Slate/Debug.h"

namespace Slate {
	nlohmann::json Filesystem::ReadJsonFile(const std::filesystem::path& path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			fprintf(stderr, "ERROR::Failed to open json file: %s\n", path.c_str());
			return {};
		}
		nlohmann::json jsonData = nlohmann::json::parse(file);
		if (jsonData.empty()) {
			fprintf(stderr, "ERROR::Json file is empty: %s\n", path.c_str());
			return {};
		}
		return jsonData;
	}
	std::string Filesystem::ReadTextFile(const std::filesystem::path& path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			fprintf(stderr, "ERROR::Failed to open text file: %s\n", path.c_str());
			return {};
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		if (buffer.str().empty()) {
			fprintf(stderr, "ERROR::Text file is empty: %s\n", path.c_str());
			return {};
		}
		return buffer.str();
	}
	std::vector<std::byte> Filesystem::ReadBinaryFile(const std::filesystem::path& path) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			fprintf(stderr, "ERROR::Failed to open binary file: %s\n", path.string().c_str());
			return {};
		}
		// get file size
		std::streampos size = file.tellg();
		if (size <= 0) {
			fprintf(stderr, "ERROR::Binary file is empty or has invalid size: %s\n", path.c_str());
			return {};
		}
		// resize and move to beginning
		std::vector<std::byte> buffer(size);
		file.seekg(0, std::ios::beg);
		// read into buffer
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			throw std::runtime_error(path.string() + ": Failed to read file - " + std::strerror(errno));
		}
		return buffer;
	}


	// if path exists
	bool Filesystem::Exists(const std::filesystem::path& path) {
		return std::filesystem::exists(path);
	}

	// creation
	void Filesystem::CreateFolder(const std::filesystem::path& path) {
		try {
			std::filesystem::create_directories(path);
		} catch (std::filesystem::filesystem_error& e) {
			throw std::runtime_error("Failed to create directory!");
		}
	}

	// simple filesystem operations
	std::string Filesystem::GetNameOfFile(const std::filesystem::path& path) {
		return path.filename().generic_string();
	}
	bool Filesystem::IsDirectoryEmpty(const std::filesystem::path& path) {
		return path.empty();
	}

	std::string Filesystem::GetRelativePath(const std::filesystem::path& path) {
		return rootFolderPath.string() + path.string();
	}
	// get directories
	std::filesystem::path Filesystem::GetProjectDirectory() {
		return rootFolderPath;
	}
	std::filesystem::path Filesystem::GetWorkingDirectory() {
		return std::filesystem::current_path();
	}
	std::filesystem::path Filesystem::GetParentDirectory(const std::filesystem::path& path) {
		std::filesystem::path parent_path = path.parent_path();
		if (parent_path.empty()) {
			throw std::runtime_error("No parent path!");
		}
		return parent_path;
	}
	std::vector<const std::filesystem::path> Filesystem::GetFoldersInDirectory(const std::filesystem::path &path) {
		std::vector<const std::filesystem::path> directories;

		const std::filesystem::directory_iterator it_end; // default construction yields past-the-end
		for (std::filesystem::directory_iterator it(path); it != it_end; ++it) {
			if (!std::filesystem::is_directory(it->status()))
				continue;
			directories.emplace_back(it->path());
		}
		return directories;
	}
	std::vector<const std::filesystem::path> Filesystem::GetFilesInDirectory(const std::filesystem::path &path) {
		std::vector<const std::filesystem::path> file_paths;

		const std::filesystem::directory_iterator it_end; // default construction yields past-the-end
		for (std::filesystem::directory_iterator it(path); it != it_end; ++it) {
			if (!std::filesystem::is_regular_file(it->status()))
				continue;
			file_paths.emplace_back(it->path());
		}
		return file_paths;
	}
}














