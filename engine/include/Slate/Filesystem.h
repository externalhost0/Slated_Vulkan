//
// Created by Hayden Rivas on 1/11/25.
//

#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Slate {
	class Filesystem {
	public:
		static std::string GetNameOfFile(const std::filesystem::path& path);

		static bool Exists(const std::filesystem::path& path);
		static bool IsDirectoryEmpty(const std::filesystem::path& path);

		static std::string GetRelativePath(const std::filesystem::path& path);
		static std::filesystem::path GetProjectDirectory();
		static std::filesystem::path GetWorkingDirectory();
		static std::filesystem::path GetParentDirectory(const std::filesystem::path& path);

		static void CreateFolder(const std::filesystem::path& path);

		static std::vector<const std::filesystem::path> GetFoldersInDirectory(const std::filesystem::path& path);
		static std::vector<const std::filesystem::path> GetFilesInDirectory(const std::filesystem::path& path);

		static nlohmann::json ReadJsonFile(const std::filesystem::path& path);
		static std::vector<std::byte> ReadBinaryFile(const std::filesystem::path& path);
		static std::string ReadTextFile(const std::filesystem::path& path);
	private:
		static inline std::filesystem::path rootFolderPath = "../../editor/";
	};
}