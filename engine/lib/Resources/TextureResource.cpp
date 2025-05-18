//
// Created by Hayden Rivas on 4/22/25.
//
#include "Slate/Resources/TextureResource.h"
#include "Slate/Common/Logger.h"
#include "Slate/Filesystem.h"

#include "Slate/Bitmap.h"

#include <volk.h>
#include <glm/glm.hpp>
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <ktx.h>

namespace Slate {

	TextureType ResolveTypeFromFileExtension(const std::string& extension) {
		static const std::unordered_map<std::string, TextureType> extensionMap = {
				// 2d types
				{ "png",     TextureType::Type_2D },
				{ "jpg",     TextureType::Type_2D },
				{ "jpeg",    TextureType::Type_2D },
				{ "tga",     TextureType::Type_2D },

				// cube maps
				{ "hdr",     TextureType::Type_Cube },
		};

		std::string ext = extension.substr(1);
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		auto it = extensionMap.find(ext);
		return it != extensionMap.end() ? it->second : TextureType::Type_2D;
	}

	Result TextureResource::_loadResourceImpl(const std::filesystem::path& path) {
		int w, h;
		this->type = ResolveTypeFromFileExtension(path.extension());
		if (type == TextureType::Type_2D) {
			void* img = stbi_load(path.c_str(), &w, &h, nullptr, 4);
			if (!img) {
				LOG_USER(LogType::Error, "Faield to load image: %d\n", path.c_str());
				stbi_image_free(img);
				return Result::FAIL;
			}
			_data = img;
			_width = w, _height = h;
			stbi_image_free(img);
		} else if (type == TextureType::Type_Cube) {
			const float* img = stbi_loadf(path.c_str(), &w, &h, nullptr, 4);
			if (!img) {
				LOG_USER(LogType::Error, "Faield to load image: %d\n", path.c_str());
				stbi_image_free((void*)img);
				return Result::FAIL;
			}
			Bitmap in(w, h, 4, BitmapFormat_Float, img);
			Bitmap out = ConvertEquirectangularMapToVerticalCross(in);
			stbi_image_free((void*)img);
			const std::string& cache_location = Filesystem::GetRelativePath("textures/cache/" + path.stem().string() + ".hdr");
			if (!stbi_write_hdr(cache_location.c_str(), out._width, out._height, out._comp, (const float*)out._data.data())) {
				LOG_USER(LogType::Error, "Failed to write HDR cache file to {}\n", cache_location.c_str());
				return Result::FAIL;
			}
			Bitmap cubemap = ConvertVerticalCrossToCubeMapFaces(out);
			_data = cubemap._data.data();
			_width = cubemap._width, _height = cubemap._height;
		} else {
			LOG_USER(LogType::Warning, "Type not supported!");
			return Result::FAIL;
		}
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