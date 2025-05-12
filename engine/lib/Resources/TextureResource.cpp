//
// Created by Hayden Rivas on 4/22/25.
//
#include "Slate/Resources/TextureResource.h"
#include "Slate/Common/Logger.h"

#include <stb_image.h>
#include <volk.h>

namespace Slate {
	Result TextureResource::LoadResourceImpl(const std::filesystem::path& path) {
		int width, height, nrChannels;
		void* data = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		if (data) {
			this->_data = std::move(data);
			this->_width = width;
			this->_height = height;
		} else {
			LOG_USER(LogType::Error, "Faield to load image: %d\n", path.c_str());
			stbi_image_free(data);
			return Result::FAIL;
		}
		stbi_image_free(data);
		return Result::SUCCESS;
	}
	void TextureResource::assignHandle(TextureHandle handle) {
		this->_handle = handle;
	}

	const void* TextureResource::getData() const {
		return _data;
	}
	VkExtent2D TextureResource::getDimensions() const {
		return {_width, _height };
	}
	TextureHandle TextureResource::getHandle() {
		return _handle;
	}
}