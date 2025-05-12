//
// Created by Hayden Rivas on 4/17/25.
//

#include "Slate/Loaders/ImageLoader.h"
#include "Slate/GX.h"
#include "Slate/Common/Logger.h"

namespace Slate {
	TextureSpec ImageLoader::loadFromPath(const std::filesystem::path& file_path) {
		int width, height, nrChannels;
		void* data = stbi_load(file_path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		VkExtent3D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		TextureSpec spec = {};
		if (data) {
			spec = {
				.data = std::move(data),
				.dimension = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) },
				.usage = TextureUsageBits::TextureUsageBits_Sampled,
				.format = VK_FORMAT_R8G8B8A8_SRGB,
			};
		} else {
			LOG_USER(LogType::Error, "Faield to load image: %d\n", file_path.c_str());
		}
		stbi_image_free(data);
		return spec;
	}
}