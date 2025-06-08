//
// Created by Hayden Rivas on 3/15/25.
//

#pragma once
#include <string>
#include <filesystem>
#include <exception>

namespace Slate {

	// a "resource" is how a file with an intended purpose is to be used throughout the engine
	// base class for all other external assets that can be stored
	// resources should ONLY contain essential storage information, that being the RAW DATA that the engine needs to access, nothing abstracted
	// these will usually filled with library types
	enum class Result : uint8_t {
		FAIL = 0,
		SUCCESS = 1
	};

	class ResourceException : public std::exception {
	public:
		ResourceException(const std::string& msg, const std::filesystem::path& path) {
			this->fullMessage = msg + " " + path.string();
		}
		const char* what() const noexcept override {
			return fullMessage.c_str();
		}
	private:
		std::string fullMessage;
	};

	struct IResource {
	public:
		IResource() = default;
		virtual ~IResource() = default;
		// each resource type needs there own way to load their data, this is implemented in LoadResourceImpl
		virtual inline Result loadResource(const std::filesystem::path& path) final {
			this->preLoad(path);
			return this->_loadResourceImpl(path);
		}
		virtual void moveResource(const std::filesystem::path& new_path) final;
		virtual void deleteResource() final;

		virtual std::string getFilename() const final { return this->filename; };
		virtual std::string getFilepath() const final { return this->filepath.string(); };
		virtual size_t getFilesize() const final { return this->filesize; };
	private:
		std::string filename;
		std::filesystem::path filepath;
		size_t filesize{0};
	protected:
		virtual Result _loadResourceImpl(const std::filesystem::path& path) = 0;
	private:
		Result preLoad(const std::filesystem::path& path);
	};

	struct AudioResource : public IResource {
	public:
	private:
		Result _loadResourceImpl(const std::filesystem::path& path) override;
	};
	struct FontResource : public IResource {
	public:
	private:
		Result _loadResourceImpl(const std::filesystem::path& path) override;
	};

}