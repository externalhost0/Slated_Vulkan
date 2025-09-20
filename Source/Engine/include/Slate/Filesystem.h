//
// Created by Hayden Rivas on 1/11/25.
//

#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Slate {
	std::filesystem::path OpenFileDialog(const std::filesystem::path& currentPath);

	class Filesystem {
	public:
		static std::string GetNameOfFile(const std::filesystem::path& path);

		static bool Exists(const std::filesystem::path& path);
		static bool IsDirectoryEmpty(const std::filesystem::path& path);

		static void SetRelativePath(const std::string& path);
		static std::string GetRelativePath(const std::filesystem::path& path);
		static std::filesystem::path GetProjectDirectory();
		static std::filesystem::path GetWorkingDirectory();
		static std::filesystem::path GetParentDirectory(const std::filesystem::path& path);

		static void CreateFolder(const std::filesystem::path& path);
		static void CreateFile(const std::filesystem::path& path);

		static void RenameFile(const std::filesystem::path& path, const std::string& name);

		static void Delete(const std::filesystem::path& path);

		static const std::vector<std::filesystem::path> GetFoldersInDirectory(const std::filesystem::path& path);
		static const std::vector<std::filesystem::path> GetFilesInDirectory(const std::filesystem::path& path);

		static nlohmann::json ReadJsonFile(const std::filesystem::path& path);
		static std::vector<std::byte> ReadBinaryFile(const std::filesystem::path& path);
		static std::string ReadTextFile(const std::filesystem::path& path);


	private:
		static inline std::filesystem::path rootFolderPath = "../../editor/";
	};
}