//
// Created by Hayden Rivas on 3/15/25.
//
#include "Slate/Resources/IResource.h"
#include "Slate/Common/Logger.h"

namespace Slate {

	Result IResource::PreLoad(const std::filesystem::path& path) {
		try {
			if (not std::filesystem::exists(path)) {
				throw ResourceException("Resource could not be found for", path);
			}
			if (std::filesystem::is_directory(path)) {
				throw ResourceException("Resource can not be a directory for", path);
			}
		} catch (const std::exception& e) {
			LOG_EXCEPTION(e);
			return Result::FAIL;
		}
		this->filepath = path;
		this->filename = path.filename();
		this->filesize = std::filesystem::file_size(path);
		return Result::SUCCESS;
	}

	void IResource::MoveResource(const std::filesystem::path& new_path) {
		try {
			if (std::filesystem::exists(this->filepath)) {
				std::filesystem::rename(this->filepath, new_path);
				this->filepath = new_path; // update the stored path
			} else {
				LOG_USER(LogType::Error, "File does not exist: {}", this->filepath.string());
			}
		} catch (const std::filesystem::filesystem_error& e) {
			LOG_EXCEPTION(e);
		}
	}
	void IResource::DeleteResource() {
		try {
			if (std::filesystem::exists(this->filepath)) {
				std::filesystem::remove(this->filepath);
			} else {
				LOG_USER(LogType::Error, "File does not exist {}", this->filepath.string());
			}
		} catch (const std::filesystem::filesystem_error& e) {
			LOG_EXCEPTION(e);
		}
	}
}