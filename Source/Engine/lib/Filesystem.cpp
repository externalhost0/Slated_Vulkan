//
// Created by Hayden Rivas on 1/10/25.
//
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <nlohmann/json.hpp>

#include "Slate/Common/HelperMacros.h"
#include "Slate/Common/Logger.h"
#include "Slate/Filesystem.h"

namespace Slate {
//	std::filesystem::path OpenFileDialog(const std::filesystem::path& currentPath) {
//		nfdu8char_t* outPath = const_cast<nfdu8char_t*>(currentPath.c_str());
//		nfdu8filteritem_t filters[] = { { "Script Files", "cs" } };
//		nfdopendialogu8args_t args = {0};
//		args.filterList = filters;
//		args.filterCount = 1;
//		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
//		if (result == NFD_OKAY) {
//			std::string newPath(outPath);
//			NFD_FreePathU8(outPath);
//			return std::filesystem::path(newPath);
//		}
//		else if (result == NFD_CANCEL) {
//			return {};
//		}
//		else {
//			LOG_USER(LogType::Error, "File dialog had an error: {}", NFD_GetError());
//			return {};
//		}
//	};

	nlohmann::json Filesystem::ReadJsonFile(const std::filesystem::path& path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			LOG_USER(LogType::Error, "Failed to open json file: {}", path.c_str());
			return {};
		}
		nlohmann::json jsonData = nlohmann::json::parse(file);
		if (jsonData.empty()) {
			LOG_USER(LogType::Warning, "Json file is empty: {}", path.c_str());
			return {};
		}
		return jsonData;
	}
	std::string Filesystem::ReadTextFile(const std::filesystem::path& path) {
		std::ifstream file(path);
		if (!file.is_open()) {
			LOG_USER(LogType::Error, "Failed to open text file: {}", path.c_str());
			return {};
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		if (buffer.str().empty()) {
			LOG_USER(LogType::Warning, "Text file is empty: {}", path.c_str());
			return {};
		}
		return buffer.str();
	}
	std::vector<std::byte> Filesystem::ReadBinaryFile(const std::filesystem::path& path) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			LOG_USER(LogType::Error, "Failed to open binary file: {}", path.c_str());
			return {};
		}
		// get file size
		std::streampos size = file.tellg();
		if (size <= 0) {
			LOG_USER(LogType::Error, "Binary file is empty or has invalid size: {}", path.c_str());
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
	void Filesystem::CreateFile(const std::filesystem::path& path) {
		try {
			std::ofstream(path).close();
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

	void Filesystem::SetRelativePath(const std::string& path) {
		rootFolderPath = path;
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
	const std::vector<std::filesystem::path> Filesystem::GetFoldersInDirectory(const std::filesystem::path &path) {
		std::vector<std::filesystem::path> directories;

		const std::filesystem::directory_iterator it_end; // default construction yields past-the-end
		for (std::filesystem::directory_iterator it(path); it != it_end; ++it) {
			if (!std::filesystem::is_directory(it->status()))
				continue;
			directories.emplace_back(it->path());
		}
		return directories;
	}
	const std::vector<std::filesystem::path> Filesystem::GetFilesInDirectory(const std::filesystem::path &path) {
		std::vector<std::filesystem::path> file_paths;

		const std::filesystem::directory_iterator it_end; // default construction yields past-the-end
		for (std::filesystem::directory_iterator it(path); it != it_end; ++it) {
			if (!std::filesystem::is_regular_file(it->status()))
				continue;
			file_paths.emplace_back(it->path());
		}
		return file_paths;
	}

	void Filesystem::RenameFile(const std::filesystem::path& path, const std::string& name) {
		try {
			std::filesystem::path newPath = path.parent_path() / name;
			std::filesystem::rename(path, newPath);
		} catch (const std::filesystem::filesystem_error& e) {
			LOG_USER(LogType::Error, "Filesystem error: ", e.what());
		}
	}
	void Filesystem::Delete(const std::filesystem::path& path) {
		if (is_directory(path)) {
			std::filesystem::remove_all(path);
		} else {
			std::filesystem::remove(path);
		}
	}
}














